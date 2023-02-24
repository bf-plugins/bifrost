# Meson build script for bifrost #
project('bifrost', 'cpp', 'cuda', 'c',
        version: '1.0.0',
        default_options: ['debug=false', 'cpp_std=c++14', 'cuda_std=c++14']
        )
fs = import('fs') 
cc = meson.get_compiler('cpp')

#--------- Load source file lists ---------#
# Note: Meson does not allow * wildcard (deliberately)
# Instead, we use file generated from command: ls src/*.cpp > meson_srclist.txt
# Note 2: map.cpp is treated separately 
srcs_cpp = fs.read('meson/srclist.txt').strip().split('\n')
srcs_cu  = fs.read('meson/srclist_cuda.txt').strip().split('\n')

# Also define the JIT sources, from which .jit files are generated
jit_srcs = ['ArrayIndexer.cuh', 
            'Complex.hpp', 
            'IndexArray.cuh',  
            'ShapeIndexer.cuh', 
            'Vector.hpp',   
            'int_fastdiv.h'
]
#------------------------------------------#

#------- Create base config.h --------#
conf_data = configuration_data()
conf_data.set('SRC_DIR', meson.source_root() / 'src')
conf_data.set('BUILD_ROOT', meson.build_root())
conf_data.set('VERSION', meson.project_version())

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
    cuda_arch_flags = cuda.nvcc_arch_flags('11', 'Auto', 
                      detected: ['5.2','6.0','6.1','7.0','7.2','7.5','8.0','8.6'])
    add_project_arguments([cuda_arch_flags], language : ['cuda'])
    
    # Create pruned static library for CUFFT
    nvprune = find_program('nvprune')
    nvcc    = find_program('nvcc')
    cuda_lib64_path = nvcc.full_path().replace('/bin/nvcc', '/lib64')

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
    'gtest',
    'psrdada'
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

if found_deps_dict.has_key('openmp') and not get_option('disable_openmp')
    conf_data.set('HAVE_OPENMP', '1')
endif
if found_deps_dict.has_key('numa') and not get_option('disable_numa')
    conf_data.set('HAVE_NUMA', '1')
endif
if found_deps_dict.has_key('hwloc') and not get_option('disable_hwloc')
    conf_data.set('HAVE_HWLOC', '1')
endif
if found_deps_dict.has_key('vma') and not get_option('disable_vma')
    conf_data.set('HAVE_VMA', '1')
endif
if found_deps_dict.has_key('psrdada') and not get_option('disable_psrdada')
    conf_data.set('HAVE_PSRDADA', '1')      # Note: not currently used
endif
#---------------------------------------#

#-------- Generate config.h ------------#
configure_file(input : 'internal/config.h.in',
               output : 'config.h',
               configuration : conf_data)
run_command('mkdir', '-p', meson.build_root() / 'bifrost', check: true)
run_command('mv', meson.build_root()/'config.h', meson.build_root()/'bifrost/config.h', check: true)
#---------------------------------------#

#--------- GENERATE CUDA JIT SOURCES --------#
if cuda_dep.found()
    # Create CUDA JIT sources
    message('generating JIT sources')
    stringify_sh = configure_file(input : 'internal/stringify.sh.in',
                    output : 'stringify.sh',
                    configuration : conf_data)

    stringify_exe = executable('stringify', 
                        ['internal/stringify.cpp'], 
                        install: true)

    jit_deps = []
    foreach jit_src:jit_srcs
        _jit_dep = custom_target(jit_src,
            input: stringify_exe,     # This line ensures stringify is compiled
            output: jit_src + '.jit',
            command: ['bash', 'stringify.sh', jit_src],
        )
        jit_deps += [_jit_dep]
    endforeach
endif
#------------------------------------------#

#--------- BUILD _device_link.o (cuFFT) ----------#
if cuda_dep.found()
    # Create pruned static library for CUFFT
    arch_flags_pruned = cuda.nvcc_arch_flags('11', '8.0')
    arch_flags_pruned += cuda_arch_flags

    cufft_pruned = custom_target('libcufft_static_pruned',
        input: cuda_lib64_path / 'libcufft_static.a',
        output: 'libcufft_static_pruned.a',
        command: [nvprune, '@INPUT@', arch_flags_pruned, '-o', '@OUTPUT@'],
        install: false,
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

    sl_cufft_device_link = custom_target('_device_link',
        input: cufft_static,
        output: '_device_link.o',
        command: [nvcc, '@INPUT@', device_link_flags, '-o', '@OUTPUT@'],
        install: false,
        install_dir:  meson.build_root()
    )
endif
#----------------------------------------#

#-------- Build bifrost library ---------#
# Compile map static library, with generated JIT sources
sl_bf_map = static_library('bf_map', 
            ['src/map.cpp'],
            include_directories: ['src'],
            build_by_default: true,
            dependencies: [cuda_dep],
            sources: jit_deps  # Makes sure stringify is run first
            )

# Now compile main bifrost library
bifrost_lib = library('bifrost', [srcs_cpp, srcs_cu, 
                                  sl_cufft_device_link, sl_bf_map],              
                     include_directories: ['src'],
                     dependencies: [cuda_dep, found_deps],
                     install: true
                   )

# Generate pkg-config file
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    bifrost_lib,
    description: 'A stream processing framework for real-time radio astronomy pipelines.',
)
#--------------------------------------#                   

#-------- Build bifrost python wrappers ---------#
if get_option('generate_python_wrapper')
    subdir('python')

    # Generate shared object
    py3.extension_module('bifrost', [srcs_cpp, srcs_cu, 
                                    sl_cufft_device_link, sl_bf_map],              
                        include_directories: ['../src'],
                        dependencies: [cuda_dep, found_deps],
                        subdir: 'bifrost',
                        install: true
                   )
    #TODO: PSRDADA GENERATION
    #TODO: CTYPESGEN MODERNIZE
endif
#--------------------------------------# 


#TODO build tests

