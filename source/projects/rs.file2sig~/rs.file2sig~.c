#include "ext_assist.h"
#include "ext_common.h"
#include "ext_mess.h"
#include "ext_obex_util.h"
#include "ext_path.h"
#include "ext_post.h"
#include "ext_proto.h"
#include "ext_sysfile.h"
#include "ext_sysmem.h"
#include "max_types.h"
#include <ext.h>
#include <ext_obex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <z_dsp.h>

static t_class *file2sig_class;

typedef struct _filebuf
{
    char *data;
    t_ptr_size length;
    bool success;
    bool free;
} t_filebuf;

typedef struct _file2sig
{
    t_pxobject x_obj;
    t_double prev_in;
    t_int read_idx;
    t_int read_shift;
    t_int byte_size;
    long polarity;
    t_filebuf file;
} t_file2sig;

static double_t BIN_AMP[2][2] = {{1.0, -1.0}, {1.0, 0.0}};
static double_t QUAD_AMP[4] = {0.5, 0.0, -0.5, -1.0};

void file2sig_assist(t_file2sig *x, void *b, long m, long a, char *s);
void *file2sig_new(t_symbol *x, long argc, t_atom *argv);
void file2sig_assist(t_file2sig *x, void *b, long msg, long arg, char *dst);
// void file2sig_float(t_file2sig *x, double f);
void file2sig_read(t_file2sig *x, t_symbol *s);
void file2sig_doread(t_file2sig *x, t_symbol *s);
void file2sig_openfile(t_file2sig *x, char *filename, short path);
void file2sig_dsp64(t_file2sig *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void file2sig_perform64(t_file2sig *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts,
                        long sampleframes, long flags, void *userparam);
void file2sig_free(t_file2sig *x);

C74_EXPORT void ext_main(void *r)
{
    t_class *c;

    c = class_new("rs.file2sig~", (method)file2sig_new, (method)file2sig_free, sizeof(t_file2sig), 0L, A_GIMME, 0);

    class_addmethod(c, (method)file2sig_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)file2sig_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)file2sig_read, "read", A_DEFSYM, 0);

    class_dspinit(c);

    CLASS_ATTR_LONG(c, "polarity", 0, t_file2sig, polarity);
    CLASS_ATTR_FILTER_CLIP(c, "polarity", 0, 1);
    CLASS_ATTR_LABEL(c, "polarity", 0, "Select bipolar/unipolar output");
    // CLASS_ATTR_DEFAULT_SAVE(c, "polarity", 0, "0");
    // CLASS_ATTR_BASIC(c, "polarity", 0);

    file2sig_class = c;

    class_register(CLASS_BOX, c);
}

void *file2sig_new(t_symbol *s, long argc, t_atom *argv)
{
    t_file2sig *x = (t_file2sig *)object_alloc(file2sig_class);

    // t_file2sig, num signal/proxy inputs
    // was z_dsp_setup
    dsp_setup((t_pxobject *)x, 1);
    outlet_new((t_pxobject *)x, "signal");

    t_symbol *filename = gensym("");

    atom_arg_getsym(&filename, 0, argc, argv);
    // necessary for @attribute syntax to work in box
    attr_args_process(x, argc, argv);

    if (filename != gensym(""))
    {
        // defer(x, (method)file2sig_doread, filename, 0, NULL);
        file2sig_read(x, filename);
    }

    x->read_idx = 0;
    x->read_shift = 0;
    x->byte_size = 8;

    return x;
}

void file2sig_assist(t_file2sig *x, void *b, long msg, long arg, char *dst)
{
    if (msg == ASSIST_OUTLET)
    {
        sprintf(dst, "(signal) Output");
    }
}

// void file2sig_float(t_file2sig *x, double f)
// {
// }

void file2sig_read(t_file2sig *x, t_symbol *s)
{
    defer(x, (method)file2sig_doread, s, 0, NULL);
}

void file2sig_doread(t_file2sig *x, t_symbol *s)
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

    file2sig_openfile(x, filename, path);
}

void file2sig_openfile(t_file2sig *x, char *filename, short path)
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

void file2sig_dsp64(t_file2sig *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, file2sig_perform64, 0, NULL);
}

void file2sig_perform64(t_file2sig *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts,
                        long sampleframes, long flags, void *userparam)
{
    double *in = ins[0];
    double *out = outs[0];
    int n = sampleframes;
    t_double value;
    bool trig = false;

    while (n--)
    {
        value = *in++;

        *out++ = BIN_AMP[x->polarity][(x->file.data[x->read_idx] >> x->read_shift) & 0b00000001];

        // set trig to true if delta
        if ((value - x->prev_in) > 0.5)
        {
            trig = true;
        }
        else
        {
            trig = false;
        }

        // increment shift if trig
        if (trig)
        {
            x->read_shift++;
            // wrap read shift
            if (x->read_shift >= x->byte_size)
                x->read_shift %= x->byte_size;

            // increment index if shift wraps
            if (x->read_shift == 0)
                x->read_idx++;
            // wrap index
            if (x->read_idx >= x->file.length)
                x->read_idx %= x->file.length;
        }

        // input into 1 sample delay line
        x->prev_in = value;
    }
}

void file2sig_free(t_file2sig *x)
{
    dsp_free((t_pxobject *)x);

    if (x->file.data != NULL)
        sysmem_freeptr(x->file.data);
}
