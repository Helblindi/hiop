#include "hiopAugLagrSubproblem.hpp"

namespace hiop
{
hiopAugLagrSubproblem::hiopAugLagrSubproblem(hiopAugLagrNlpAdapter *nlp_AugLagr_in):
  subproblem(nlp_AugLagr_in),
  ipoptApp(nullptr),
  hiopSolver(nullptr),
  hiopWrapper(nullptr),
  _subproblemStatus(NotInitialized),
  _solverStatus(NlpSolve_IncompleteInit),
  _EPS_TOL_OPTIM(1e-6)
{
}

hiopAugLagrSubproblem::~hiopAugLagrSubproblem()
{
  if (ipoptApp)   delete ipoptApp;
  if (hiopSolver) delete hiopSolver;
  if (hiopWrapper) delete hiopWrapper;
}

/** Tolerance set by the main Aug Lagr solver, the subproblem
 * tolerance is gradually tightened as the solution is approached
 * in outer AL iterations. */
void hiopAugLagrSubproblem::setTolerance(double tol)
{
  _EPS_TOL_OPTIM = tol;
}

/** Initializes the subproblem solver and keeps the solver so that it is ready
 *  to be used also for the following subproblem solutions. This prevents
 *  the creation/destruction of the subproblem solver in every major iteration.
 *  We also assume that the major iterations may be restarted with different
 *  solver, or run multiple times so we keep all the previously created solvers */
void hiopAugLagrSubproblem::initialize()
{
  //check subproblem options
  //_EPS_TOL_ACCEP = subproblem->options->GetNumeric("acceptable_tolerance");
  //_MAX_N_IT      = subproblem->options->GetInteger("max_iter");
  
  if(subproblem->options->GetString("subproblem_solver")=="ipopt")
  {
    // test if ipopt was initialized previously
    if(_subproblemStatus != IpoptInitialized &&  _subproblemStatus != IpoptHiopInitialized)
    {
      //create IPOPT
      ipoptApp = IpoptApplicationFactory();

      //set IPOPT options
      //ipoptApp->Options()->SetNumericValue("tol", 1e-9);
      //ipoptApp->Options()->SetStringValue("mu_strategy", "adaptive");
      //ipoptApp->Options()->SetStringValue("output_file", "ipopt.out");
      ipoptApp->Options()->SetIntegerValue("print_level", subproblem->options->GetInteger("verbosity_level_subproblem") );
      ipoptApp->Options()->SetStringValue("sb","yes"); //disables Ipopt welcome message

      // Intialize the IpoptApplication and process the options
      ApplicationReturnStatus st = ipoptApp->Initialize();
      if (st != Solve_Succeeded) {
        subproblem->log->printf(hovError, "\n\n*** Error during IPOPT initialization!\n Error %d\n", st);
      }
      
      //update the Subproblem status 
      _subproblemStatus = static_cast<hiopSubproblemStatus>(
         static_cast<int>(_subproblemStatus) + 
         static_cast<int>(IpoptInitialized));
    }
  }
  else if(subproblem->options->GetString("subproblem_solver")=="hiop")
  {
    // test if hiop was initialized previously
    if(_subproblemStatus != HiopInitialized && _subproblemStatus != IpoptHiopInitialized)
    {
      //create HIOP wrapper of the problem
      hiopWrapper = new hiopNlpDenseConstraints(*subproblem); 

      // set HIOP options
      hiopWrapper->options->SetNumericValue("tolerance", _EPS_TOL_OPTIM); 
      hiopWrapper->options->SetStringValue("fixed_var", "relax"); //remove fails
      hiopWrapper->options->SetIntegerValue("verbosity_level", subproblem->options->GetInteger("verbosity_level_subproblem"));

      //create and initialize the HiopSolver
      hiopSolver = new hiopAlgFilterIPM(hiopWrapper);

      //update the Subproblem status 
      _subproblemStatus = static_cast<hiopSubproblemStatus>(
         static_cast<int>(_subproblemStatus) + 
         static_cast<int>(HiopInitialized));
    }
  }
  
  _solverStatus = NlpSolve_SolveNotCalled;
}

hiopSolveStatus hiopAugLagrSubproblem::solveSubproblem_ipopt()
{
  checkConsistency();

  ipoptApp->Options()->SetNumericValue("tol", _EPS_TOL_OPTIM);

  //adaptive performs much better than monotone
  ipoptApp->Options()->SetStringValue("mu_strategy", "adaptive");

  //for the first N subproblems use exact hessian, then use only QN
  static int subproblem_iter = 0;
  const int SWITCH_IT = subproblem->options->GetInteger("quasi_newton_switch_iter");
  //negative switch_iter disables QN
  if (SWITCH_IT >= 0 && subproblem_iter > SWITCH_IT) 
  {
    assert(false);
      subproblem->log->printf(hovWarning, "hipAugLagrSubproblem: switching to the Quasi-Newton mode!\n");
      ipoptApp->Options()->SetStringValue("hessian_approximation", "limited-memory");
      ipoptApp->Options()->SetIntegerValue("limited_memory_max_history", 6);
      ipoptApp->Options()->SetNumericValue("limited_memory_init_val", 1e6 );
      ipoptApp->Options()->SetStringValue("limited_memory_initialization", "constant" );
      ipoptApp->Options()->SetIntegerValue("print_level", 5);

      //warm start the QN
      ipoptApp->Options()->SetStringValue("warm_start_init_point", "yes");
      // warm starting with Cosmin
      //bound_push 1e-8 or 1e-24
      //bound_frac 1e-8
      //slack_bound_push 1e-8
      //slack_bound_frac 1e-8
      //bound_relax_factor 0
      //ipoptApp->Options()->SetNumericValue("bound_push", 1e-16);
      //ipoptApp->Options()->SetNumericValue("bound_frac", 1e-16);
      ipoptApp->Options()->SetNumericValue("mu_init", 1e-8);
      ipoptApp->Options()->SetNumericValue("mu_target", 1e-8);
      ipoptApp->Options()->SetStringValue("mu_strategy", "monotone");
 
  }
  subproblem_iter++;

  // Ask Ipopt to solve the problem
  ApplicationReturnStatus st = ipoptApp->OptimizeTNLP(subproblem);
  displayTerminationMsgIpopt(st);

  if (st == Solve_Succeeded) { return Solve_Success; }
  else { return UnknownNLPSolveStatus; }

}

hiopSolveStatus hiopAugLagrSubproblem::solveSubproblem_hiop()
{
  checkConsistency();

  hiopWrapper->options->SetNumericValue("tolerance", _EPS_TOL_OPTIM);

  // Ask HiOP to solve the problem
  hiopSolveStatus st = hiopSolver->run();
  displayTerminationMsgHiop(st);

  return st;
}

/** Solves the subproblem using the specified solver */
hiopSolveStatus hiopAugLagrSubproblem::run()
{
  _solverStatus = NlpSolve_Pending;

  if(subproblem->options->GetString("subproblem_solver")=="ipopt")
    _solverStatus = solveSubproblem_ipopt();
  else if(subproblem->options->GetString("subproblem_solver")=="hiop")
    _solverStatus = solveSubproblem_hiop();
  
  // subproblem->log->printf(hovSummary, "Subproblem: Converged in %d iterations.\n", getNumIterations());

  return _solverStatus;
}

/** Check consistency of the current object state vs required options
 *  specified in the subproblem, i.e. if the required solver was initialized */
void hiopAugLagrSubproblem::checkConsistency() const
{
  if(subproblem->options->GetString("subproblem_solver")=="ipopt")
  {
    if (_subproblemStatus != IpoptInitialized && _subproblemStatus != IpoptHiopInitialized)
    {
      subproblem->log->printf(hovError, "hipAugLagrSubproblem consistency check: Ipopt solver not initialized, please call initialize() first!\n");
    }
  }
  else if(subproblem->options->GetString("subproblem_solver")=="hiop")
  {
    if(_subproblemStatus != HiopInitialized && _subproblemStatus != IpoptHiopInitialized)
    {
      subproblem->log->printf(hovError, "hipAugLagrSubproblem consistency check: Hiop solver not initialized, please call initialize() first!\n");
    }
  }
}

/* returns the objective value; valid only after 'run' method has been called */
double hiopAugLagrSubproblem::getObjective() const
{
  if(_solverStatus==NlpSolve_IncompleteInit || _solverStatus == NlpSolve_SolveNotCalled)
    subproblem->log->printf(hovError, "getObjective: hiOp AugLagr subproblem did not initialize entirely or the 'run' function was not called.");
  if(_solverStatus==NlpSolve_Pending)
    subproblem->log->printf(hovWarning, "getObjective: hiOp AL subproblem does not seem to have completed yet. The objective value returned may not be optimal.");
  
  //TODO
  assert(0);
  return 100.;
}

/* returns the primal vector x; valid only after 'run' method has been called */
void hiopAugLagrSubproblem::getSolution(double* x) const
{
  if(_solverStatus==NlpSolve_IncompleteInit || _solverStatus == NlpSolve_SolveNotCalled)
    subproblem->log->printf(hovError, "getSolution: hiOp AugLagr subproblem did not initialize entirely or the 'run' function was not called.");
  if(_solverStatus==NlpSolve_Pending)
    subproblem->log->printf(hovWarning, "getSolution: hiOp AugLagr subproblem have not completed yet. The primal vector returned may not be optimal.");

  checkConsistency();
  if(subproblem->options->GetString("subproblem_solver")=="ipopt")
  {
    //IpoptApplication doesn't provide a method to access the solution.
    //The solution is passed to user in callback finalize_solution which
    //is implemented in AugLagrNlpAdapter, the solution is thus cached there 
    subproblem->get_ipoptSolution(x);
  }
  else if(subproblem->options->GetString("subproblem_solver")=="hiop")
  {
    hiopSolver->getSolution(x);
  }

  return;
}

/* returns the primal vector x; valid only after 'run' method has been called */
void hiopAugLagrSubproblem::getSolution_duals(double* zL, double* zU) const
{
  if(_solverStatus==NlpSolve_IncompleteInit || _solverStatus == NlpSolve_SolveNotCalled)
    subproblem->log->printf(hovError, "getSolution: hiOp AugLagr subproblem did not initialize entirely or the 'run' function was not called.");
  if(_solverStatus==NlpSolve_Pending)
    subproblem->log->printf(hovWarning, "getSolution: hiOp AugLagr subproblem have not completed yet. The primal vector returned may not be optimal.");

  checkConsistency();
  if(subproblem->options->GetString("subproblem_solver")=="ipopt")
  {
    //IpoptApplication doesn't provide a method to access the solution.
    //The solution is passed to user in callback finalize_solution which
    //is implemented in AugLagrNlpAdapter, the solution is thus cached there 
    subproblem->get_ipoptBoundMultipliers(zL, zU);
  }
  else if(subproblem->options->GetString("subproblem_solver")=="hiop")
  {
    hiopSolver->getSolution_duals(zL, zU);
  }
  return;
}

/* returns the status of the solver */
hiopSolveStatus hiopAugLagrSubproblem::getSolveStatus() const
{
  return _solverStatus;
}

/* returns the number of iterations */
int hiopAugLagrSubproblem::getNumIterations() const
{
  if(_solverStatus==NlpSolve_IncompleteInit || _solverStatus == NlpSolve_SolveNotCalled)
    subproblem->log->printf(hovError, "getNumIterations: hiOp did not initialize entirely or the 'run' function was not called.");
  if(_solverStatus==NlpSolve_Pending)
    subproblem->log->printf(hovWarning, "getNumIterations: hiOp does not seem to have completed yet. The objective value returned may not be optimal.");
 
  checkConsistency();
  if(subproblem->options->GetString("subproblem_solver")=="hiop")
  {
      return hiopSolver->getNumIterations();
  }
  
  if(subproblem->options->GetString("subproblem_solver")=="ipopt")
  {
      return subproblem->get_ipoptNumIters(); 
  }

  return -1;
}

/***** Termination message *****/
void hiopAugLagrSubproblem::displayTerminationMsgIpopt(ApplicationReturnStatus st) {
   switch (st)  {
    case Solve_Succeeded:
      { subproblem->log->printf(hovSummary, "Subproblem: Successfull termination.\n"); //%s\n", subproblem->runStats.getSummary().c_str());
        break;
      }
    case Solved_To_Acceptable_Level:
      { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Solved_To_Acceptable_Level\n");
        break;
      }
    case Infeasible_Problem_Detected:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Infeasible_Problem_Detected\n");     
        break;
      }
    case Search_Direction_Becomes_Too_Small:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Search_Direction_Becomes_Too_Small\n");      
        break;
      }
    case User_Requested_Stop:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was User_Requested_Stop\n");     
        break;
      }
    case Feasible_Point_Found:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Feasible_Point_Found\n");    
        break;
      }
    case Maximum_Iterations_Exceeded:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Maximum_Iterations_Exceeded\n");     
        break;
      }
    case Restoration_Failed:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Restoration_Failed\n");      
        break;
      }
    case Error_In_Step_Computation:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Error_In_step_Computation\n");       
        break;
      }
    case Maximum_CpuTime_Exceeded:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Maximum_CpuTime_Exceeded\n");        
        break;
      }
    case Invalid_Option:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Invalid_Options\n");  
        break;
      }
    case Invalid_Number_Detected:
       { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return code was Invalid_Number_Detected\n");         
        break;
      }
    default:
      { subproblem->log->printf(hovWarning, "Subproblem: Ipopt return an iternal error %d\n", st); 
        break;
      }
   }
}



void hiopAugLagrSubproblem::displayTerminationMsgHiop(hiopSolveStatus st) {

  switch(st) {
  case Solve_Success: 
    {
      subproblem->log->printf(hovSummary, "Subproblem: Successfull termination.\n"); //%s\n", subproblem->runStats.getSummary().c_str());
      break;
    }
  case Solve_Success_RelTol: 
    {
      subproblem->log->printf(hovWarning, "Subproblem: Successfull termination (error within the relative tolerance).\n");
      break;
    }
  case Solve_Acceptable_Level:
    {
      subproblem->log->printf(hovWarning, "Subproblem: Solve to only to the acceptable tolerance(s).\n");
      break;
    }
  case Max_Iter_Exceeded:
    {
      subproblem->log->printf(hovWarning, "Subproblem: Maximum number of iterations reached.\n");
      break;
    }
  case Steplength_Too_Small:
    {
      subproblem->log->printf(hovWarning, "Subproblem: Couldn't solve the problem.\n");
      subproblem->log->printf(hovWarning, "Subproblem: Linesearch returned unsuccessfully (small step). Probable cause: inaccurate gradients/Jacobians or infeasible problem.\n");
      break;
    }
  case User_Stopped:
    {
      subproblem->log->printf(hovWarning, "Subproblem: Stopped by the user through the user provided iterate callback.\n");
      break;
    }
  default:
    {
      subproblem->log->printf(hovWarning, "Subproblem: Do not know why hiop stopped. This shouldn't happen. :)\n");
      assert(false && "Do not know why hiop stopped. This shouldn't happen.");
      break;
    }
  };
}

}
