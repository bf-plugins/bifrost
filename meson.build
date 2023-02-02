# Meson build script for pacer imager
project('bifrost', 'cpp', 'cuda', 
        version: '1.0.0')

# Basic setup -- We should be able to just add
#    default_options: ['debug=true', 'cpp_std=c++14']
# to the project() call, but nvcc does not appear to get these passed on
c_std = 'c++14'
buildtype = 'debug'
comp_flags = ['-g', '-std=c++14', '-DBF_CUDA_ENABLED=1']
add_project_arguments(comp_flags, language : ['c', 'cpp', 'cuda'])

# Load list of files to compile into shared object
# Note: Meson does not allow * wildcard (deliberately)
# Instead, we use file generated from command: ls src/*.cpp > meson_srclist.txt
fs = import('fs') 
srcs_cpp = fs.read('meson_srclist.txt').strip().split('\n')
srcs_cu  = fs.read('meson_srclist_cuda.txt').strip().split('\n')

# Dependencies: CUDA
cuda = import('unstable-cuda')
cuda_flags = cuda.nvcc_arch_flags('11', '6.0, 6.1, 7.0')
#cuda_flags2 = ['-DBF_CUDA_ENABLED=1']
add_project_arguments([cuda_flags], language : ['cuda'])

cuda_dep = dependency('cuda', version : '>=11', 
                      modules : ['cudart', 'nvrtc', 'cufft', 'curand'])

openmp_dep = dependency('openmp')

# Subprojects
gtest_dep = dependency('gtest')  # Wrap file exists

# Build library (libbifrost)
bifrost_lib = library('bifrost', [srcs_cpp, srcs_cu],              
                     include_directories: ['src'],
                     dependencies: [cuda_dep, gtest_dep, openmp_dep],
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