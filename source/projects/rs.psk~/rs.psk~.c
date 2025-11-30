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
    t_double prev_in;
    t_double bpsk_amp[2];
    t_double qpsk_amp[4];
    t_int read_idx;
    t_int read_shift;
    t_int byte_size;
    t_filebuf file;
} t_psk;

void psk_assist(t_psk *x, void *b, long m, long a, char *s);
void *psk_new(t_symbol *x, long argc, t_atom *argv);
void psk_assist(t_psk *x, void *b, long msg, long arg, char *dst);
// void psk_float(t_psk *x, double f);
void psk_read(t_psk *x, t_symbol *s);
void psk_doread(t_psk *x, t_symbol *s);
void psk_openfile(t_psk *x, char *filename, short path);
void psk_dsp64(t_psk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void psk_perform64(t_psk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes,
                   long flags, void *userparam);
void psk_free(t_psk *x);

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

    x->read_idx = 0;
    x->read_shift = 0;
    x->byte_size = 8;

    x->bpsk_amp[0] = 1.0;
    x->bpsk_amp[1] = -1.0;

    x->qpsk_amp[0] = 0.5;
    x->qpsk_amp[1] = 0.0;
    x->qpsk_amp[2] = -0.5;
    x->qpsk_amp[3] = -1.0;

    return x;
}

void psk_assist(t_psk *x, void *b, long msg, long arg, char *dst)
{
    if (msg == ASSIST_OUTLET)
    {
        sprintf(dst, "(signal) Output");
    }
}

// void psk_float(t_psk *x, double f)
// {
// }

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

void psk_dsp64(t_psk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, psk_perform64, 0, NULL);
}

void psk_perform64(t_psk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes,
                   long flags, void *userparam)
{
    double *in = ins[0];
    double *out = outs[0];
    int n = sampleframes;
    t_double value;

    while (n--)
    {
        value = *in++;

        // *out++ = (double)x->read_shift / 8.0;
        // *out++ = fabs(value - x->prev_in);
        *out++ = x->bpsk_amp[(x->file.data[x->read_idx] >> x->read_shift) & 0b00000001];

        // increment shift if delta
        if (fabs(value - x->prev_in) > 0.5)
            x->read_shift++;
        // post("%d\n", x->read_shift);
        // wrap read shift
        if (x->read_shift >= x->byte_size)
            x->read_shift %= x->byte_size;
        // increment index if shift wraps
        if (x->read_shift == 0)
            x->read_idx++;
        // wrap index
        if (x->read_idx >= x->file.length)
            x->read_idx %= x->file.length;

        // while (x->read_shift >= x->byte_size)
        //     x->read_shift -= x->byte_size;
        // while (x->read_shift < 0)
        //     x->read_shift += x->byte_size;
        //
        // if (x->read_shift == 0)
        //     x->read_idx++;
        //
        // while (x->read_idx >= x->file.length)
        //     x->read_idx -= x->file.length;
        // while (x->read_idx < 0)
        //     x->read_idx += x->file.length;

        x->prev_in = value;
    }
}

void psk_free(t_psk *x)
{
    dsp_free((t_pxobject *)x);

    if (x->file.data != NULL)
        sysmem_freeptr(x->file.data);
}
