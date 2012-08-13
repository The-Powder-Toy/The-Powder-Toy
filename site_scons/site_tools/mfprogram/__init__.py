#
# Copyright (c) 2010 Western Digital Corporation
# Alan Somers asomers (at) gmail (dot) com
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

####
import os
import new
import sys
## 
import SCons

if sys.version_info < (2,6,0):
    from relpath import relpath
else:
    from os.path import relpath


def sc_relpath(src, destdir):
    """Like relpath but aware of SCons convention regarding '#' in pathnames"""
    if src[0] == '#':
        return relpath(src[1:], destdir)
    else:
        return relpath(os.path.join(destdir, src), destdir)



class MF_Executor(SCons.Executor.Executor):
    """Custom Executor that can scan each target file for its dependencies
    individually, rather than giving every target the same deps.
    Assumes that there is a one-to-one relationship between sources and targets
    and the targets have the same basenames as their respective sources
    ie. [[foo.o, bar.o], [foo.c, bar.c]]"""
    def scan(self, scanner, node_list):
        tgt_names = [os.path.splitext(
                        os.path.basename(str(i)))[0] for i in self.targets]
        env = self.get_build_env()

        if scanner:
            for node in node_list:
                tgt = \
                   self.targets[tgt_names.index(
                                    os.path.splitext(
                                        os.path.basename(str(node)))[0])]
                node.disambiguate()
                s = scanner.select(node)
                if not s:
                    continue
                path = self.get_build_scanner_path(s)
                tgt.add_to_implicit(node.get_implicit_deps(env, s, path))
        else:
            kw = self.get_kw()
            for node in node_list:
                tgt = \
                   self.targets[tgt_names.index(
                                    os.path.splitext(
                                        os.path.basename(str(node)))[0])]
                node.disambiguate()
                scanner = node.get_env_scanner(env, kw)
                if not scanner:
                    continue
                scanner = scanner.select(node)
                if not scanner:
                    continue
                path = self.get_build_scanner_path(scanner)
                tgt.add_to_implicit(node.get_implicit_deps(env, scanner, path))


def MF_get_single_executor(self, env, tlist, slist, executor_kw):
    if not self.action:
        raise UserError, "Builder %s must have an action to build %s." % \
                (self.get_name(env or self.env), map(str,tlist))
    return MF_Executor(self.action, env, [], tlist, slist, executor_kw)

def exists(env):
    return env.WhereIs(env.subst['$CC']) or env.WhereIs(env.subst['$CXX'])

def MFProgramEmitter(target, source, env):
    """Ensures that target list is complete, and does validity checking.  Sets precious"""
    if len(target) == 1 and len(source) > 1:
        #Looks like the user specified many sources and SCons created 1 target
        #targets are implicit, but the builder doesn't know how to handle 
        #suffixes for multiple target files, so we'll do it here
        objdir = env.get('OBJDIR', '')
        #target = [os.path.join(
        #    objdir, 
        #        os.path.splitext(
        #            os.path.basename(str(i)))[0] + '.o' ) for i in source]
    elif len(source) == 1 and 'OBJDIR' in env:
        target = os.path.join(
            env['OBJDIR'], 
                os.path.splitext(
                    os.path.basename(str(source[0])))[0] + '.o' )
    else:
        #targets are explicit, we need to check their validity
        tgt_names = [os.path.splitext(
                        os.path.basename(str(i)))[0] for i in target]
        src_names = [os.path.splitext(
                        os.path.basename(str(i)))[0] for i in source]
        tgt_dirs = [os.path.dirname(str(i)) for i in target]
        if sorted(tgt_names) != sorted(src_names):
            raise ValueError, "target files do not have obvious one-one relationship to source files"
        if len(set(src_names)) != len(src_names):
            raise ValueError, "source files may not include identically named files in different directories"
        if len(set(tgt_dirs)) != 1:
            raise ValueError, "Target files must all be in same directory"

    for t in target:
        env.Precious(t)
    return target, source

def MFProgramGenerator(source, target, env, for_signature):
    #Rebuild everything if 
    #   a) the number of dependencies has changed
    #   b) any target does not exist
    #   c) the build command has changed
    #Else rebuild only those c files that have changed_since_last_build
    #The signature of this builder should always be the same, because the
    #multifile compile is always functionally equivalent to rebuilding
    #everything
    
    if for_signature:
        pared_sources = source
    else:
        #First a sanity check
        assert len(set([os.path.splitext(str(i))[1] for i in source])) == 1, \
                "All source files must have the same extension."
        pared_sources = []
        src_names = [os.path.splitext(os.path.basename(str(i)))[0] 
                        for i in source]
        tgt_names = [os.path.splitext(os.path.basename(str(t)))[0] 
                        for t in target]
        ni = target[0].get_binfo()
        oi = target[0].get_stored_info().binfo
        if ni.bactsig != oi.bactsig:
            #Command line has changed
            pared_sources = source
        else:
            for i in range(len(tgt_names)):
                t = target[i]
                tgt_name = tgt_names[i]
                if not t.exists():
                    #a target does not exist
                    pared_sources = source
                    break
                bi = t.get_stored_info().binfo
                then = bi.bsourcesigs + bi.bdependsigs + bi.bimplicitsigs
                children = t.children()
                if len(children) != len(then):
                    #the number of dependencies has changed
                    pared_sources = source
                    break
                for child, prev_ni in zip(children, then):
                    if child.changed_since_last_build(t, prev_ni) and \
                        not t in pared_sources:
                        #If child is a source file, not an explicit or implicit
                        #dependency, then it is not truly a dependency of any target
                        #except that with the same basename.  This is a limitation
                        #of SCons.node, which assumes that all sources of a Node
                        #are dependencies of all targets.  So we check for that case
                        #here and only rebuild as necessary.
                        src_name = os.path.splitext(os.path.basename(str(child)))[0]
                        if src_name not in tgt_names or src_name == tgt_name:
                            s = source[src_names.index(tgt_name)]
                            pared_sources.append(s)
    assert len(pared_sources) > 0
    destdir = str(target[0].dir)
    #finding sconscript_dir is a bit of a hack.  It assumes that the source
    #files are always going to be in the same directory as the SConscript file
    #which is not necessarily true.  BUG BY Alan Somers
    sconscript_dir = os.path.dirname(str(pared_sources[0]))
    prefixed_sources = [relpath(str(i), destdir) for i in pared_sources]
    prefixed_sources_str = ' '.join([str(i) for i in prefixed_sources])
    lang_ext = os.path.splitext(prefixed_sources[0])[1]
    tgt_names2 = [os.path.splitext(os.path.basename(str(t)))[0] 
                for t in target]

    _CPPPATH = []
    if 'CPPPATH' in env:
        for i in env['CPPPATH']:
            #if i[0] == '#':
            ##_CPPPATH.append(relpath(i[1:], destdir))
            _CPPPATH.append(i)
            #else:
            #    _CPPPATH.append(relpath(os.path.join(sconscript_dir, i), 
            #                            destdir))

    defines = ""
    for t in env['CPPDEFINES']:
        defines += ("-D"+str(t)+" ")

    _CPPINCFLAGS = ['-I' + i for i in _CPPPATH]
    _CCOMCOM = '$CPPFLAGS $_CPPDEFFLAGS $defines %s' % ' '.join(_CPPINCFLAGS)

    libstr = ""
    for t in env['LIBS']:
        libstr += ("-l"+t+" ")

    if lang_ext == '.c' :
        _CCCOM = 'cd %s && $CC $CFLAGS $CCFLAGS %s %s $LINKFLAGS %s -o %s' % \
                    (destdir, _CCOMCOM, prefixed_sources_str, libstr, tgt_names2[0])
        #XXX BUG BY Alan Somers.  $CCCOMSTR gets substituted using the full list of target files,
        #not prefixed_sources
        cmd = SCons.Script.Action(env.subst(_CCCOM), "$CCCOMSTR")
    elif lang_ext in ['.cc', '.cpp']:
        _CXXCOM = 'cd %s && $CXX $CXXFLAGS $CCFLAGS %s %s $LINKFLAGS %s -o %s' % \
                    (destdir, _CCOMCOM, prefixed_sources_str, libstr, tgt_names2[0])
        cmd = SCons.Script.Action(env.subst(_CXXCOM), "$CXXCOMSTR")
    else:
        assert False, "Unknown source file extension %s" % lang_ext
    return cmd

def generate(env):
    """Adds the MFObject builder to your environment"""
    MFProgramBld = env.Builder(generator = MFProgramGenerator, 
                              emitter = MFProgramEmitter,
                              suffix = '.o', 
                              source_scanner=SCons.Tool.SourceFileScanner)
    MFProgramBld.get_single_executor = new.instancemethod(MF_get_single_executor, 
                        MFProgramBld, MFProgramBld.__class__)

    env.Append(BUILDERS = {'MFProgram': MFProgramBld})