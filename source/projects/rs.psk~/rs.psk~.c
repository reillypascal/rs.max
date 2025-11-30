#include "ext_assist.h"
#include "ext_common.h"
#include "ext_mess.h"
#include "ext_path.h"
#include "ext_post.h"
#include "ext_proto.h"
#include "ext_sysfile.h"
#include "ext_sysmem.h"
#include <ext.h>
#include <ext_obex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <z_dsp.h>

static t_class *psk_class;

typedef struct _filebuf
{
    char *data;
    unsigned long length;
    bool success;
    bool free;
} t_filebuf;

typedef struct _psk
{
    t_pxobject x_obj;
    t_sample x_val;
    t_filebuf file;
} t_psk;

void psk_assist(t_psk *x, void *b, long m, long a, char *s);
void *psk_new(t_symbol *x, long argc, t_atom *argv);
void psk_dsp64(t_psk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void psk_assist(t_psk *x, void *b, long msg, long arg, char *dst);
void psk_float(t_psk *x, double f);
void psk_doread(t_psk *x, t_symbol *s);
void psk_read(t_psk *x, t_symbol *s);
void psk_openfile(t_psk *x, char *filename, short path);
void psk_free(t_psk *x);
void psk_perform64_method(t_psk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts,
                          long sampleframes, long flags, void *userparam);

C74_EXPORT void ext_main(void *r)
{
    t_class *c;

    c = class_new("rs.psk~", (method)psk_new, (method)psk_free, sizeof(t_psk), 0L, A_GIMME, 0);

    class_addmethod(c, (method)psk_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)psk_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)psk_read, "read", A_DEFSYM, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    psk_class = c;
}

void *psk_new(t_symbol *s, long argc, t_atom *argv)
{
    t_psk *x = (t_psk *)object_alloc(psk_class);

    // t_psk, num signal/proxy inputs
    // was z_dsp_setup
    dsp_setup((t_pxobject *)x, 1);
    outlet_new((t_pxobject *)x, "signal");

    t_symbol *filename = gensym("");

    atom_arg_getsym(&filename, 0, argc, argv);

    if (filename != gensym(""))
    {
        // defer(x, (method)psk_doread, filename, 0, NULL);
        psk_read(x, filename);
    }

    return x;
}

void psk_dsp64(t_psk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
}

void psk_assist(t_psk *x, void *b, long msg, long arg, char *dst)
{
    if (msg == ASSIST_OUTLET)
    {
        sprintf(dst, "(signal) Output");
    }
}

void psk_float(t_psk *x, double f)
{
}

void psk_read(t_psk *x, t_symbol *s)
{
    defer(x, (method)psk_doread, s, 0, NULL);
}

void psk_doread(t_psk *x, t_symbol *s)
{
    // TODO: empty string of 4 char for filetype?
    // right now it seems to work, but it's uninitialized
    t_fourcc filetype, outtype;
    short numtypes = 1;
    char filename[MAX_PATH_CHARS];
    short path;

    if (s == gensym(""))
    {
        // 0 for any filetype permitted
        if (open_dialog(filename, &path, &outtype, &filetype, 0))
            return;
    }
    else
    {
        strcpy(filename, s->s_name);
        // 0 for any filetype permitted
        if (locatefile_extended(filename, &path, &outtype, &filetype, 0))
        {
            object_error((t_object *)x, "%s: not found", s->s_name);
            return;
        }
    }

    psk_openfile(x, filename, path);
}

void psk_openfile(t_psk *x, char *filename, short path)
{
    t_filehandle fh;

    if (path_opensysfile(filename, path, &fh, READ_PERM))
    {
        object_error((t_object *)x, "error opening %s", filename);
        return;
    }

    sysfile_geteof(fh, &x->file.length);
    x->file.data = sysmem_newptr(x->file.length);

    sysfile_read(fh, &x->file.length, x->file.data);

    // for (int i = 0; i < x->file.length; i++)
    // {
    //     post("%u\n", x->file.data[i]);
    // }

    sysfile_close(fh);
}

void psk_free(t_psk *x)
{
    dsp_free((t_pxobject *)x);
    if (x->file.data != NULL)
        sysmem_freeptr(x->file.data);
}

void psk_perform64_method(t_psk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts,
                          long sampleframes, long flags, void *userparam)
{
}
