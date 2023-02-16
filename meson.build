# Meson build script for pacer imager
project('bifrost', 'cpp', 'cuda', 'c',
        version: '1.0.0',
        default_options: ['debug=false', 'cpp_std=c++14', 'cuda_std=c++14']
        )

# Basic setup -- We should be able to just add
#    default_options: ['debug=true', 'cpp_std=c++14']
# to the project() call, but nvcc does not appear to get these passed on
c_std = 'c++14'
buildtype = 'release'
comp_flags = ['-DBF_CUDA_ENABLED=1']
add_project_arguments(comp_flags, 
    language : ['c', 'cpp', 'cuda']
    )

cc = meson.get_compiler('cpp')


# Load list of files to compile into shared object
# Note: Meson does not allow * wildcard (deliberately)
# Instead, we use file generated from command: ls src/*.cpp > meson_srclist.txt
fs = import('fs') 
srcs_cpp = fs.read('meson_srclist.txt').strip().split('\n')
srcs_cu  = fs.read('meson_srclist_cuda.txt').strip().split('\n')
srcs_cu_jit  = fs.read('meson_srclist_cuda_jit.txt').strip().split('\n')

# Dependencies: CUDA
cuda = import('unstable-cuda')
cuda_flags = cuda.nvcc_arch_flags('11', '6.0,6.1')
cuda_flags2 = ['-Xcompiler', '-march=native', 
               '-Xcompiler', '-fPIC -DPIC', '-dc']
add_project_arguments([cuda_flags, cuda_flags2], language : ['cuda'])

cuda_dep = dependency('cuda', version : '>=11', 
                      modules : ['cudart', 'nvrtc', 'culibos',
                                 'cublas',  'curand'])

openmp_dep = dependency('openmp')


r = run_command('nvprune', 
            '/usr/local/cuda/lib64/libcufft_static.a',
            cuda_flags, 
            '-o', meson.build_root()+'/'+'libcufft_static_pruned.a'
            )
#cufft_static = cc.find_library('cufft_static', dirs: '/usr/local/cuda/lib64')
cufft_pruned = cc.find_library('cufft_static_pruned', dirs: meson.build_root())


#------- Create config.h --------#
conf_data = configuration_data()
# Memory alignment
conf_data.set('ALIGNMENT', '4096')

# CUDA support
conf_data.set('HAVE_CUDA', '1')
conf_data.set('CUDA_VERSION', '11.0')
conf_data.set('GPU_ARCHS', '60 61')
conf_data.set('GPU_MIN_ARCH', '60')
conf_data.set('GPU_MAX_ARCH', '61')
conf_data.set('GPU_SHAREDMEM', '16384')
conf_data.set('GPU_PASCAL_MANAGEDMEM', '1')
conf_data.set('MAP_KERNEL_STDCXX', 'c++11')
conf_data.set('HAVE_MAP_CACHE', '1')
conf_data.set('PACKAGE_VERSION_MAJOR', '1')
conf_data.set('PACKAGE_VERSION_MINOR', '0')

# Features
conf_data.set('HAVE_FLOAT128', '0')
conf_data.set('HAVE_OPENMP', '1')
conf_data.set('HAVE_NUMA', '1')
conf_data.set('HAVE_HWLOC', '1')
conf_data.set('HAVE_VMA', '0')

# Debugging features
conf_data.set('HAVE_DEBUG', '0')
conf_data.set('HAVE_TRACE', '0')
conf_data.set('HAVE_CUDA_DEBUG', '0')

# Logging directory
conf_data.set('HAVE_TMPFS', '/dev/shm/bifrost')

# Apply config
configure_file(input : 'src/bifrost/config.h.in',
               output : 'config.h',
               configuration : conf_data)

#-------------------------------#

# Subprojects
gtest_dep = dependency('gtest')  # Wrap file exists
stringify = subproject('stringify')



#foreach jit_src : srcs_cu_jit
#  r = run_command('stringify.sh', jit_src,  
#              capture: true, check: true)
#endforeach

# Build library (libbifrost)
bifrost_lib = library('bifrost', [srcs_cpp, srcs_cu],              
                     include_directories: ['src', 'src/bifrost'],
                     dependencies: [cuda_dep, gtest_dep, openmp_dep, cufft_pruned],
                     install: true
                    )

#TODO build tests, make optional

#echo "Building CUDA source file fft.cu"
#g++ -x c++ -MT fft.o -MMD -MP -MF .deps/fft.Td -std=c++14 -O3 -Wall -pedantic -fopenmp -DBF_CUDA_ENABLED=1 -march=native  -fPIC -DPIC  -DBF_CUDA_ENABLED=1 -I. -I. -I/usr/local/cuda/include -E fft.cu > /dev/null
#/usr/local/cuda/bin/nvcc -std=c++11 -O3 -Xcompiler "-Wall" -DBF_CUDA_ENABLED=1 -Xcompiler "-march=native" -Xcompiler " -fPIC -DPIC"  -gencode arch=compute_60,\"code=sm_60\"  -gencode arch=compute_61,\"code=sm_61\" -gencode arch=compute_61,\"code=compute_61\" -DBF_CUDA_ENABLED=1 -I. -I. -I/usr/local/cuda/include -Xcompiler "-fmessage-length=80 "  -c -o fft.o fft.cu
#mv -f .deps/fft.Td .deps/fft.d
# Note: This needs to be compiled with "-dc" to make CUFFT callbacks work
#/usr/local/cuda/bin/nvcc -std=c++11 -O3 -Xcompiler "-Wall" -DBF_CUDA_ENABLED=1 -Xcompiler "-march=native" -Xcompiler " -fPIC -DPIC"  -gencode arch=compute_60,\"code=sm_60\"  -gencode arch=compute_61,\"code=sm_61\" -gencode arch=compute_61,\"code=compute_61\" -DBF_CUDA_ENABLED=1 -I. -I. -I/usr/local/cuda/include -Xcompiler "-fmessage-length=80 " -dc -o fft_kernels.o fft_kernels.cu

# Create pkg-config file
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    bifrost_lib,
    description: 'A stream processing framework for real-time radio astronomy pipelines.',
)

# Subdirectories 
# subdir('examples')
# subdir('include')
