# Meson build script for BIFROST
project('bifrost', 'cpp', 'cuda', 'c',
        version: '1.0.0',
        default_options: ['debug=false', 'cpp_std=c++14', 'cuda_std=c++14']
        )

c_std = 'c++14'        
buildtype = 'release'
comp_flags = [ '-std=c++14', '-DBF_CUDA_ENABLED=1']
add_project_arguments(comp_flags,
    language : ['c', 'cpp', 'cuda']
    )

cc = meson.get_compiler('cpp')

#--------- Load source file lists ---------#
# Note: Meson does not allow * wildcard (deliberately)
# Instead, we use file generated from command: ls src/*.cpp > meson_srclist.txt
fs = import('fs') 
srcs_cpp = fs.read('meson_srclist.txt').strip().split('\n')
srcs_cu  = fs.read('meson_srclist_cuda.txt').strip().split('\n')
srcs_cu_jit  = fs.read('meson_srclist_cuda_jit.txt').strip().split('\n')
#------------------------------------------#

#------- Create base config.h --------#
conf_data = configuration_data()
# Memory alignment
conf_data.set('ALIGNMENT', '4096')
# Features
conf_data.set('HAVE_FLOAT128', '0')
conf_data.set('HAVE_OPENMP', '0')
conf_data.set('HAVE_NUMA', '0')
conf_data.set('HAVE_HWLOC', '0')
conf_data.set('HAVE_VMA', '0')
# Debugging features
# TODO let these be set
conf_data.set('HAVE_DEBUG', '0')
conf_data.set('HAVE_TRACE', '0')
conf_data.set('HAVE_CUDA_DEBUG', '0')
# Logging directory
conf_data.set('HAVE_TMPFS', '/dev/shm/bifrost')
#------------------------------------#

#------------ CUDA SETUP ------------#
cuda = import('unstable-cuda')
cuda_dep = dependency('cuda', version : '>=11', 
                      modules : ['cudart', 'nvrtc', 'culibos', 'cublas', 'curand'])

if cuda_dep.found()
    # TODO un-hardcode architecutre flags
    cuda_arch_flags = cuda.nvcc_arch_flags('11', 'Auto', 
                      detected: ['5.2','6.0','6.1','7.0','7.2','7.5','8.0','8.6'])
    add_project_arguments([cuda_arch_flags], language : ['cuda'])
    
    # Create pruned static library for CUFFT
    nvprune = find_program('nvprune')
    nvcc    = find_program('nvcc')
    cuda_lib64_path = nvcc.full_path().replace('/bin/nvcc', '/lib64')
    arch_flags_pruned = cuda.nvcc_arch_flags('11', '8.0')
    arch_flags_pruned += cuda_arch_flags

    cufft_pruned = custom_target('libcufft_static_pruned',
        input: cuda_lib64_path / 'libcufft_static.a',
        output: 'libcufft_static_pruned.a',
        command: [nvprune, '@INPUT@', arch_flags_pruned, '-o', '@OUTPUT@'],
        install: true,
        install_dir:  meson.build_root()
    )

    # Now, compile cuFFT static library
    cufft_static = static_library('fft_kernels', 
               ['src/fft_kernels.cu', 'src/fft.cu'],
               cuda_args: ['-dc'], 
               include_directories: ['src'],
               override_options: ['buildtype=release'],
               build_by_default: true).extract_all_objects(recursive: false)
    
    # Next create _device_link.o
    device_link_flags = ['-dlink', 
                         '-Xcompiler', '"-march=native"',
                         '-Xcompiler', '-fPIC', 
                         '-Xcompiler', '-DPIC', 
                         '-L.', '-lcufft_static_pruned']
    device_link_flags += cuda_arch_flags

    cufft_device_link = custom_target('_device_link',
        input: cufft_static,
        output: '_device_link.o',
        command: [nvcc, '@INPUT@', device_link_flags, '-o', '@OUTPUT@'],
        install: true,
        install_dir:  meson.build_root()
    )

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
else
    conf_data.set('HAVE_CUDA', '0')
endif
#---------------------------------------#

#--------- Optional dependencies -------#
libnames = [
    'hwloc', 
    'numa', 
    'openmp', 
    'mlx4', 
    'ibverbs',
    'gtest'
    ]

found_deps = []
found_deps_dict = {}
foreach libname:libnames
	lib = dependency('lib' + libname, required:false)
	if not lib.found()
		lib = cc.find_library(libname, required:false)
	endif
	if lib.found()
		found_deps += [ lib ]
        found_deps_dict += {libname: lib}
	else
        message('Did not find ', libname)
	endif
endforeach

if found_deps_dict.has_key('openmp')
    conf_data.set('HAVE_OPENMP', '1')
endif
if found_deps_dict.has_key('numa')
    conf_data.set('HAVE_NUMA', '1')
endif
if found_deps_dict.has_key('hwloc')
    conf_data.set('HAVE_HWLOC', '1')
endif
if found_deps_dict.has_key('vma')
    conf_data.set('HAVE_VMA', '1')
endif
#---------------------------------------#


#TODO: get JIT stringify working
stringify     = subproject('stringify')
stringify_exe = find_program('stringify')
#foreach jit_src : srcs_cu_jit
#  r = run_command('stringify.sh', jit_src,  
#              capture: true, check: true)
#endforeach



# Generate config.h file (prebuild step)
configure_file(input : 'src/bifrost/config.h.in',
               output : 'config.h',
               configuration : conf_data)
#-------------------------------#


# Build library (libbifrost)
bifrost_lib = library('bifrost', [srcs_cpp, srcs_cu, cufft_device_link],              
                     include_directories: ['src'],
                     dependencies: [cuda_dep, found_deps],
                     install: true,
                     override_options: ['buildtype=release']
                    )

#TODO build tests, make optional

# Create pkg-config file
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    bifrost_lib,
    description: 'A stream processing framework for real-time radio astronomy pipelines.',
)

# Subdirectories 
# subdir('examples')
# subdir('include')
