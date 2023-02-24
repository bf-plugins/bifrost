py_mod = import('python')
py3 = py_mod.find_installation()
py3_dep = py3.dependency()


py_conf_data = configuration_data()
py_conf_data.set('PYTHON', py3.path())
py_conf_data.set('BUILD_ROOT', meson.build_root())
py_conf_data.set('SRC_DIR', conf_data.get('SRC_DIR'))
py_conf_data.set('INCLUDE_DIR', conf_data.get('INCLUDE_DIR'))
py_conf_data.set('VERSION', conf_data.get('VERSION'))

bf_py_ctypesgen = configure_file(input : '../internal/run_ctypesgen.sh.in',
                output : 'run_ctypesgen.sh',
                configuration : py_conf_data)


bf_py_version = configure_file(input : '../internal/version.py.in',
                output : 'version.py',
                configuration : py_conf_data)

# TODO: undo stringify_exe hack
# TODO: make ctypesgen raise error if it doesn't compile properly
bf_py_libbifrost = custom_target('libbifrost_generated',
                                    input: stringify_exe,
                                    output: 'libbifrost_generated.py',
                                    command: ['bash', 'python/run_ctypesgen.sh'],
                                    install: true,
                                    install_dir: py3.get_install_dir(subdir: 'bifrost'))

python_sources = fs.read('../meson/python_srclist.txt').strip().split('\n')
py3.install_sources([python_sources, bf_py_version], 
                    subdir: 'bifrost')

bf_py_subdirs = ['blocks', 'telemetry', 'views', 'version']
foreach bf_py_subdir:bf_py_subdirs
    subdir_filelist = fs.read('../meson/python_'+bf_py_subdir+'_srclist.txt').strip().split('\n')
    py3.install_sources(subdir_filelist, subdir: 'bifrost/'+bf_py_subdir)
endforeach

### PSRDADA - TODO
if found_deps_dict.has_key('psrdada') and not get_option('disable_psrdada')
    #PSRHOME ?= /usr/local
    ##PSRDADA_PYTHON_BINDINGS_FILE = bifrost/libpsrdada_generated.py
    #PSRDADA_HEADERS = \
    #	$(PSRHOME)/include/dada_hdu.h \
    #	$(PSRHOME)/include/ipcio.h \
    #	$(PSRHOME)/include/ipcbuf.h \
    #	$(PSRHOME)/include/multilog.h
    #
    #all: build
endif
