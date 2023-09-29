module use -a /usr/workspace/hiop/software/spack_modules_20230927/linux-rhel8-broadwell/

module purge

module load gcc/12.1.1

# cmake@=3.26.3%gcc@=12.1.1~doc+ncurses+ownlibs build_system=generic build_type=Release arch=linux-rhel8-broadwell
module load cmake/3.26.3-gcc-12.1.1-lfjjtqg
# blt@=0.4.1%gcc@=12.1.1 build_system=generic arch=linux-rhel8-broadwell
module load blt/0.4.1-gcc-12.1.1-ik42vx2
# gmake@=4.4.1%gcc@=12.1.1~guile build_system=autotools arch=linux-rhel8-broadwell
module load gmake/4.4.1-gcc-12.1.1-p7abhqz
# camp@=0.2.3%gcc@=12.1.1~cuda~ipo+openmp~rocm~tests build_system=cmake build_type=Release generator=make arch=linux-rhel8-broadwell
module load camp/0.2.3-gcc-12.1.1-ttwiwm3
# berkeley-db@=18.1.40%gcc@=12.1.1+cxx~docs+stl build_system=autotools patches=26090f4,b231fcc arch=linux-rhel8-broadwell
module load berkeley-db/18.1.40-gcc-12.1.1-qcdrdlx
# libiconv@=1.17%gcc@=12.1.1 build_system=autotools libs=shared,static arch=linux-rhel8-broadwell
module load libiconv/1.17-gcc-12.1.1-u3uuyht
# diffutils@=3.9%gcc@=12.1.1 build_system=autotools arch=linux-rhel8-broadwell
module load diffutils/3.9-gcc-12.1.1-rap2lqr
# bzip2@=1.0.8%gcc@=12.1.1~debug~pic+shared build_system=generic arch=linux-rhel8-broadwell
module load bzip2/1.0.8-gcc-12.1.1-lblcvjr
# pkgconf@=1.9.5%gcc@=12.1.1 build_system=autotools arch=linux-rhel8-broadwell
module load pkgconf/1.9.5-gcc-12.1.1-3d74jmk
# ncurses@=6.4%gcc@=12.1.1~symlinks+termlib abi=none build_system=autotools arch=linux-rhel8-broadwell
module load ncurses/6.4-gcc-12.1.1-5tyumf7
# readline@=8.2%gcc@=12.1.1 build_system=autotools patches=bbf97f1 arch=linux-rhel8-broadwell
module load readline/8.2-gcc-12.1.1-y3ewlvg
# gdbm@=1.23%gcc@=12.1.1 build_system=autotools arch=linux-rhel8-broadwell
module load gdbm/1.23-gcc-12.1.1-co6vwcc
# zlib-ng@=2.1.3%gcc@=12.1.1+compat+opt build_system=autotools patches=299b958,ae9077a,b692621 arch=linux-rhel8-broadwell
module load zlib-ng/2.1.3-gcc-12.1.1-o46r76j
# perl@=5.38.0%gcc@=12.1.1+cpanm+opcode+open+shared+threads build_system=generic patches=714e4d1 arch=linux-rhel8-broadwell
module load perl/5.38.0-gcc-12.1.1-luhhgnv
# openblas@=0.3.24%gcc@=12.1.1~bignuma~consistent_fpcsr+fortran~ilp64+locking+pic+shared build_system=makefile symbol_suffix=none threads=none arch=linux-rhel8-broadwell
module load openblas/0.3.24-gcc-12.1.1-wukhnd2
# coinhsl@=2015.06.23%gcc@=12.1.1+blas build_system=autotools arch=linux-rhel8-broadwell
module load coinhsl/2015.06.23-gcc-12.1.1-xrkf6oj
# metis@=5.1.0%gcc@=12.1.1~gdb~int64~ipo~real64+shared build_system=cmake build_type=Release generator=make patches=4991da9,93a7903,b1225da arch=linux-rhel8-broadwell
module load metis/5.1.0-gcc-12.1.1-dmc7b2d
# mvapich2@=2.3.7%gcc@=12.1.1~alloca~cuda~debug~hwloc_graphics~hwlocv2+regcache+wrapperrpath build_system=autotools ch3_rank_bits=32 fabrics=mrail file_systems=auto patches=d98d8e7 process_managers=auto threads=multiple arch=linux-rhel8-broadwell
module load mvapich2/2.3.7-gcc-12.1.1-odpkaro
# raja@=0.14.0%gcc@=12.1.1~cuda~examples~exercises~ipo+openmp~rocm+shared~tests build_system=cmake build_type=Release generator=make arch=linux-rhel8-broadwell
module load raja/0.14.0-gcc-12.1.1-kzbomqi
# umpire@=6.0.0%gcc@=12.1.1~c~cuda~device_alloc~deviceconst~examples~fortran~ipo~numa+openmp~rocm~shared build_system=cmake build_type=Release generator=make tests=none arch=linux-rhel8-broadwell
module load umpire/6.0.0-gcc-12.1.1-feqvxct


#export EXTRA_CMAKE_ARGS="$EXTRA_CMAKE_ARGS -DHIOP_CTEST_LAUNCH_COMMAND:STRING='jsrun -n 2 -a 1 -c 1 -g 1'"
export EXTRA_CMAKE_ARGS="$EXTRA_CMAKE_ARGS -DHIOP_USE_RAJA:STRING=ON"
export CMAKE_CACHE_SCRIPT=gcc-cpu.cmake





