
/*
 
 factorOracle, a MAX external
 Adam James Wilson
 awilson@citytech.cuny.edu
 
 LICENSE:
 
 This software is copyrighted by Adam James Wilson and others. The following terms (the "Standard Improved BSD License") apply:
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */


// NOTE: This is an alpha version still under development. There may be bugs.




#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"
#include "ext_critical.h"
#include "ext_path.h"
#include "ext_sysfile.h"




typedef struct _state
{

	long suffixLink;
	long numberOfTransitionElements;
    long transitionElement;
    long *transitionEndStates;
	
} t_state;




typedef struct _factorOracle
{

	t_object ob;
    void *m_outlet1;
    void *m_outlet2;
    void *m_outlet3;
    void *m_outlet4;
    void *m_outlet5;
    void *m_outlet6;
    void *m_outlet10;    
	long *alphabet;
    char *json;
	t_state *states;
    long *input_string;
    long input_limit;
    long input_index;
    long output_state;
    long *output_string;
    long output_limit;
    long output_index;
    long default_size;
    short send_metadata;
	double probability;
    long m_in1;
    void *m_proxy1;
    long m_in2;
    void *m_proxy2;
    long mode;
    long previousRoute;
} t_factorOracle;




static const char *MEMORY_ALLOCATION_ERROR = "Unable to allocate memory.";
static const char *FILE_WRITE_ERROR = "An error occured while attempting to write the file.";
static const char *EMPTY_ORACLE_ERROR = "The oracle is empty.";

void *factorOracle_new(t_symbol *s, long argc, t_atom *argv);
void factorOracle_free(t_factorOracle *x);
void factorOracle_assist(t_factorOracle *x, void *b, long m, long a, char *s);

void factorOracle_bang(t_factorOracle *x);
void factorOracle_int(t_factorOracle *x, long transition);
void factorOracle_float(t_factorOracle *x, double ft);

void factorOracle_mode(t_factorOracle *x, long option);
void factorOracle_alphabet(t_factorOracle *x);
void factorOracle_instring(t_factorOracle *x);
void factorOracle_outstring(t_factorOracle *x);
void factorOracle_clear(t_factorOracle *x);

void factorOracle_oracle(t_factorOracle *x);

void factorOracle_openfile(t_factorOracle *x, char *filename, short path);
void factorOracle_doread(t_factorOracle *x, t_symbol *s, short argc, t_atom *argv);
void factorOracle_read(t_factorOracle *x, t_symbol *s);

void factorOracle_writefile(t_factorOracle *x, char *filename, short path, short in_or_out);
void factorOracle_dowrite(t_factorOracle *x, t_symbol *s, short argc, t_atom *argv);
void factorOracle_writealphabet(t_factorOracle *x, t_symbol *s);
void factorOracle_writeinput(t_factorOracle *x, t_symbol *s);
void factorOracle_writeoutput(t_factorOracle *x, t_symbol *s);
void factorOracle_writejson(t_factorOracle *x, t_symbol *s);

void factorOracle_probability(t_factorOracle *x, double probability);
long factorOracle_walk(t_factorOracle *x);

long memberOfTransitionElements(long transition, long k, t_factorOracle *x);
long buildOracle(long transition, t_factorOracle *x);
int getInputString(t_factorOracle *x);
void getStateInfo(t_factorOracle *x, long state_index);
long getAlphabet(t_factorOracle *x);
long json(t_factorOracle *x);

long mode_1(t_factorOracle *x);
long mode_2(t_factorOracle *x);
long mode_3(t_factorOracle *x);

void *factorOracle_class;




void ext_main(void *r)
{
    t_class *c;
	
	c = class_new("factorOracle", (method)factorOracle_new, 
		(method)factorOracle_free, (long)sizeof(t_factorOracle), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)factorOracle_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)factorOracle_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)factorOracle_read, "read", A_DEFSYM, 0);
    class_addmethod(c, (method)factorOracle_writealphabet, "writealphabet", A_DEFSYM, 0);
    class_addmethod(c, (method)factorOracle_writeinput, "writeinput", A_DEFSYM, 0);
    class_addmethod(c, (method)factorOracle_writeoutput, "writeoutput", A_DEFSYM, 0);
    class_addmethod(c, (method)factorOracle_writejson, "writejson", A_DEFSYM, 0);
	class_addmethod(c, (method)factorOracle_int, "int", A_LONG, 0);
	class_addmethod(c, (method)factorOracle_bang, "bang", A_DEFER, 0);
	class_addmethod(c, (method)factorOracle_clear, "clear", A_NOTHING, 0);
    class_addmethod(c, (method)factorOracle_mode, "mode", A_LONG, 0);
    class_addmethod(c, (method)factorOracle_oracle, "oracle", A_NOTHING, 0);
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	factorOracle_class = c;
}




void *factorOracle_new(t_symbol *s, long argc, t_atom *argv)
{
	t_factorOracle *x = NULL;

	if ((x = (t_factorOracle *)object_alloc(factorOracle_class))) {
		object_post((t_object *)x, "A new %s object was instantiated: 0x%X.", s->s_name, x);
        
        x->m_proxy1 = proxy_new((t_object *)x, 1, &x->m_in1);
        x->m_proxy2 = proxy_new((t_object *)x, 2, &x->m_in2);
        x->m_outlet1  =  intout((t_object *)x);
        x->m_outlet2  =  intout((t_object *)x);
        x->m_outlet3  =  intout((t_object *)x);
        x->m_outlet4  = listout((t_object *)x);
        x->m_outlet5  = listout((t_object *)x);
        x->m_outlet6  =  intout((t_object *)x);
        x->m_outlet10 =  intout((t_object *)x);
        
		x->input_index = 0;
        x->output_index = 0;
        x->output_state = -1;
        x->default_size = 10000;
        
        if (argc >= 1 && ((argv)->a_type == A_LONG) && (atom_getlong(argv) > -1))
        {
            x->states = (t_state*)sysmem_newptr(atom_getlong(argv+0) * sizeof(t_state));
            x->input_limit = atom_getlong(argv+0);
            x->output_string = (long*)sysmem_newptr((atom_getlong(argv+0)) * sizeof(long));
            x->output_limit = atom_getlong(argv+0);

            object_post((t_object *)x, "Number of states allocated for input: %ld.", atom_getlong(argv+0));
        }
        else
        {
            x->states = (t_state*)sysmem_newptr(x->default_size * sizeof(t_state));
            x->input_limit = x->default_size;
            x->output_string = (long*)sysmem_newptr(x->default_size * sizeof(long));
            x->output_limit = x->default_size;
            object_post((t_object *)x, "Argument 1 must be an integer greater than 0 specifying the number of input states. Allocating default: %ld.", x->default_size);
        }
        
        if (argc > 1)
        {
            if ((argv+1)->a_type == A_SYM)
            {
                factorOracle_read(x, atom_getsym(argv+1));
            }
            else
            {
                object_error((t_object *)x, "Input file '%s' failed to load.", atom_getsym(argv+1));
            }
        }
        
        if (argc > 2)
        {
            object_post((t_object *)x, "Ignoring extra arguments.");
        }
        
        x->mode = 1;
        x->probability = 0.75;
	}
    
	srand((unsigned)time(NULL));
	return (x);
}




void factorOracle_free(t_factorOracle *x)
{
    sysmem_freeptr(x->alphabet);
    sysmem_freeptr(x->input_string);
    sysmem_freeptr(x->output_string);
    sysmem_freeptr(x->json);
	for (long i = 0; i < x->input_index; i ++)
    {
		sysmem_freeptr(x->states[i].transitionEndStates);
	}
	sysmem_freeptr(x->states);
    sysmem_freeptr(x->m_proxy1);
    sysmem_freeptr(x->m_proxy2);
}




void factorOracle_assist(t_factorOracle *x, void *b, long m, long a, char *s)
{

	if (m == ASSIST_INLET)
    {
		switch (a)
        {
			case 0:
                sprintf(s, "bang causes output. Many message options. See help documentation for more detail.");
                break;
            case 1:
                sprintf(s, "int sets state without causing output, bang gets state information. See help documentation for more detail.");
                break;
			case 2:
                sprintf(s, "Float [ 0.0 - 1.0 ]; specifies the probability of congruence between input and output strings.");
                break;
		}
	}
	else
    {
        switch (a) {
            case 0:
                sprintf(s, "transition chosen on bang received in leftmost inlet.");
                break;
            case 1:
                sprintf(s, "state connected by suffix link from current state.");
                break;
            case 2:
                sprintf(s, "end states of transitions from current state");
                break;
            case 3:
                sprintf(s, "transitions from current state.");
                break;
            case 4:
                sprintf(s, "number of transitions from current state.");
                break;
            case 5:
                sprintf(s, "current state.");
                break;
            case 6:
                sprintf(s, "final state.");
                break;
        }
	}
}




long memberOfTransitionElements(long transition, long k, t_factorOracle *x)
{
	long test = -1;
	long i;
	for (i = 0; i < x->states[k].numberOfTransitionElements; i++)
    {
		if (transition == x->states[(x->states[k].transitionEndStates[i]) - 1].transitionElement)
        {
			test = i;
			break;
		}
	}
	return test;
}




long buildOracle(long transition, t_factorOracle *x)
{
    x->states[x->input_index].transitionElement = transition;
    x->states[x->input_index].transitionEndStates = (long*)sysmem_newptr(sizeof(long));
    if (x->states[x->input_index].transitionEndStates == NULL)
    {
        object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
        return -1;
    }
    
    x->states[x->input_index].transitionEndStates[0] = (x->input_index + 1);
    x->states[x->input_index].numberOfTransitionElements = 1;

	long k;
	if (x->input_index == 0)
    {
		x->states[x->input_index].suffixLink = -1;
		k = -1; 
	}
    else
    {
		k = x->states[x->input_index].suffixLink;
	}

	while ((k != -1) && ((memberOfTransitionElements(transition, k, x)) == -1))
    {
		x->states[k].transitionEndStates = (long*)sysmem_resizeptr(x->states[k].transitionEndStates, (x->states[k].numberOfTransitionElements + 1) * sizeof(long));
        if (x->states[k].transitionEndStates == NULL)
        {
            object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
            return -1;
        }
        
		x->states[k].transitionEndStates[x->states[k].numberOfTransitionElements] = (x->input_index + 1);
        x->states[k].numberOfTransitionElements += 1;
		k = x->states[k].suffixLink;
	}

	if (k == -1)
    {
		x->states[x->input_index+1].suffixLink = 0;
	}
    else
    {
        x->states[(x->input_index + 1)].suffixLink = x->states[k].transitionEndStates[memberOfTransitionElements(transition, k, x)];
	}
    
    x->states[(x->input_index + 1)].numberOfTransitionElements = 0;
    
    return 0;
}




void getStateInfo(t_factorOracle *x, long state_index)
{
    critical_enter(0);
    
    x->output_state = state_index;
    
    t_atom *es;
    t_atom *et;
    long len;
    
    if (x->output_state == x->input_index)
    {
        es = (t_atom*)sysmem_newptr(sizeof(t_atom));
        et = (t_atom*)sysmem_newptr(sizeof(t_atom));
        if (es == NULL || et == NULL)
        {
            critical_exit(0);
            return;
        }
        len = 1;
        atom_setlong(es, -1);
        atom_setlong(et, -1);
    }
    else
    {
        len = x->states[x->output_state].numberOfTransitionElements;
        es = (t_atom*)sysmem_newptr(len * sizeof(t_atom));
        et = (t_atom*)sysmem_newptr(len * sizeof(t_atom));
        if (es == NULL || et == NULL)
        {
            critical_exit(0);
            return;
        }
        long endState;
        for (long i = 0; i < x->states[x->output_state].numberOfTransitionElements; i++)
        {
            endState = x->states[x->output_state].transitionEndStates[i];
            atom_setlong(es+i, endState);
            atom_setlong(et+i, x->states[endState - 1].transitionElement);
        }
    }
    critical_exit(0);
    outlet_int( x->m_outlet1, x->input_index);
    outlet_int( x->m_outlet2, x->output_state);
    outlet_int( x->m_outlet3, x->states[x->output_state].numberOfTransitionElements);
    outlet_list(x->m_outlet4, NULL, len, et);
    outlet_list(x->m_outlet5, NULL, len, es);
    outlet_int( x->m_outlet6, x->states[x->output_state].suffixLink);
    sysmem_freeptr(es);
    sysmem_freeptr(et);
}





void chooseTransition(t_factorOracle *x)
{
     critical_enter(0);
     long output;
     if (x->output_index < x->output_limit)
     {
         switch (x->mode)
         {
             case 1:
                 output = mode_1(x);
                 break;
             case 2:
                 output = mode_2(x);
                 break;
             case 3:
                 output = mode_3(x);
                 break;
         }
         x->output_string[x->output_index] = output;
         x->output_index += 1;
     }
     else
     {
         object_error((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
         critical_exit(0);
         return;
     }
     critical_exit(0);
     outlet_int(x->m_outlet10, output);
}





void getCurrentStateInfo(t_factorOracle *x)
{
    if (x->output_state > -1)
    {
        getStateInfo(x, x->output_state);
    }
}




void factorOracle_bang(t_factorOracle *x)
{
    switch (proxy_getinlet((t_object *)x))
    {
        case 0:
            chooseTransition(x);
            break;
        case 2:
            getCurrentStateInfo(x);
            break;
    }
}




void addTransition(t_factorOracle *x, long transition)
{
    critical_enter(0);
    if (x->input_index < x->input_limit)
    {
        if (buildOracle(transition, x) != 0)
        {
            object_error((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
            critical_exit(0);
            return;
        }
        x->input_index += 1;
    }
    else
    {
        object_error((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
    }
    critical_exit(0);
}




void changeState(t_factorOracle *x, long state_index)
{
    if (state_index < 0 || state_index > x->input_index)
    {
        object_post((t_object *)x, "State index %ld is outside of index range [0, %ld].", state_index, x->input_index);
    }
    else
    {
        getStateInfo(x, state_index);
    }
}




void factorOracle_int(t_factorOracle *x, long n)
{
    switch (proxy_getinlet((t_object *)x))
    {
        case 0:
        {
            addTransition(x, n);
            break;
        }
        case 2:
        {
            changeState(x, n);
            break;
        }
    }
}




void factorOracle_float(t_factorOracle *x, double n)
{
    switch (proxy_getinlet((t_object *)x))
    {
        case 1:
        {
            factorOracle_probability(x, n);
            break;
        }
    }
}




int getInputString(t_factorOracle *x)
{
    sysmem_freeptr(x->input_string);

    x->input_string = (long*)sysmem_newptr(x->input_index * sizeof(long));
    if (x->input_string == NULL) {
        object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
        return -1;
    }
    
	for (long i = 0; i < x->input_index; i++)
    {
        x->input_string[i] = x->states[i].transitionElement;
	}
    
    return 0;
}




void factorOracle_oracle(t_factorOracle *x)
{
    critical_enter(0);
    long i, j;
	for (i = 0; i < x->input_index; i++)
    {
		long nte = x->states[i].numberOfTransitionElements;
		long suf = x->states[i].suffixLink;
		post("State %ld numberOfTransitionElements = %ld.", i, nte);
		post("State %ld suffixLink points to state %ld.", i, suf);
        long trn;
        long end;
		for (j = 0; j < nte; j++)
        {
            trn = x->states[(x->states[i].transitionEndStates[j]) - 1].transitionElement;
			end = (x->states[i].transitionEndStates[j]);
            post("    State %ld transition element %ld points to state %ld.", i, trn, end);
		}
	}
	long fnte = x->states[x->input_index].numberOfTransitionElements;
	long fsuf = x->states[x->input_index].suffixLink;
	post("Final state (%ld) numberOfTransitionElements = %ld.", i, fnte);
	post("Final state (%ld) suffixLink points to state %ld.", i, fsuf);
	post("input_index = %ld", x->input_index);
    critical_exit(0);
}




void factorOracle_clear(t_factorOracle *x)
{
    critical_enter(0);

    sysmem_freeptr(x->alphabet);
    sysmem_freeptr(x->input_string);
    sysmem_freeptr(x->json);
	
	for (long i = 0; i < x->input_index; i ++) {
		sysmem_freeptr(x->states[i].transitionEndStates);
        x->states[i].numberOfTransitionElements = 0;
	}

	x->input_index = 0;
    x->output_index = 0;
    x->output_state = -1;

    critical_exit(0);
}




int compare(const void *a, const void *b)
{
    long x = *(const long *)a;
    long y = *(const long *)b;
    
    if (x < y)
    {
        return -1;
    }
    else if (x > y)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}




long getAlphabet(t_factorOracle *x)
{
    sysmem_freeptr(x->alphabet);
    getInputString(x);
    
    long *tmp = (long*)sysmem_newptrclear(x->input_index * sizeof(long));
    if (tmp == NULL)
    {
        object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
        return -1;
    }
    
    long i;
    for (i = 0; i < x->input_index; i++)
    {
        tmp[i] = x->input_string[i];
    }
    
    qsort(tmp, x->input_index, sizeof(long), compare);
    
    x->alphabet = (long*)sysmem_newptrclear(sizeof(long));
    if (x->alphabet == NULL)
    {
        object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
        return -1;
    }
    unsigned long alphabet_index = 0;
    for (i = 0; i < x->input_index - 1; i++)
    {
        if (tmp[i] != tmp[i + 1])
        {
            x->alphabet[alphabet_index] = tmp[i];
            alphabet_index = alphabet_index + 1;
            x->alphabet = (long*)sysmem_resizeptr(x->alphabet, (alphabet_index + 1) * sizeof(long));
            if (x->alphabet == NULL)
            {
                object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
                return -1;
            }
        }
    }
    x->alphabet[alphabet_index] = tmp[x->input_index - 1];
    
    sysmem_freeptr(tmp);
    sysmem_freeptr(x->input_string);
    
    return alphabet_index + 1;
}




long json(t_factorOracle *x)
{
    unsigned long MAX_LONG_CHARS;
    if (sizeof(long) == 4)
    {
        MAX_LONG_CHARS = 12;
    }
    else
    {
        MAX_LONG_CHARS = 21;
    }
     x->json = sysmem_newptr(4 * x->input_index * MAX_LONG_CHARS * CHAR_BIT + 24 * x->input_index * CHAR_BIT - 2 * MAX_LONG_CHARS * CHAR_BIT - 2 * CHAR_BIT);
    if (x->json == NULL)
    {
        object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
        return -1;
    }
    
    unsigned long tmpbuff_size = (CHAR_BIT * (MAX_LONG_CHARS * 2 + 6)), tcb = (CHAR_BIT * 2);
    char *tmpbuff = sysmem_newptr(tmpbuff_size);
    if (x->json == NULL)
    {
        object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
        return -1;
    }
    
    unsigned long len, i, j, json_next_index = 0;
    long end_state, transition;
    
    len = snprintf(tmpbuff, tcb, "{\n");
    memcpy(x->json+json_next_index, tmpbuff, len);
    json_next_index += len;
    for (i = 0; i <= x->input_index; i++)
    {
        if (i > 0)
        {
            len = snprintf(tmpbuff, tcb, ",\n");
            memcpy(x->json+json_next_index, tmpbuff, len);
            json_next_index += len;
        }
        len = snprintf(tmpbuff, tmpbuff_size, "\"%ld\":[{", i);
        memcpy(x->json+json_next_index, tmpbuff, len);
        json_next_index += len;
        for (j = 0; j < x->states[i].numberOfTransitionElements; j++)
        {
            if (j > 0)
            {
                len = snprintf(tmpbuff, CHAR_BIT, ",");
                memcpy(x->json+json_next_index, tmpbuff, len);
                json_next_index += len;
            }
            end_state = x->states[i].transitionEndStates[j];
            transition = x->states[end_state - 1].transitionElement;
            len = snprintf(tmpbuff, tmpbuff_size, "\"%ld\":\"%ld\"", end_state, transition);
            memcpy(x->json+json_next_index, tmpbuff, len);
            json_next_index += len;
        }
        len = snprintf(tmpbuff, tmpbuff_size, "},\"%ld\"]", x->states[i].suffixLink);
        memcpy(x->json+json_next_index, tmpbuff, len);
        json_next_index += len;
    }
    len = snprintf(tmpbuff, tcb, "\n}");
    memcpy(x->json+json_next_index, tmpbuff, len);
    json_next_index += len;

    sysmem_freeptr(tmpbuff);
    return json_next_index;
}




void factorOracle_writefile(t_factorOracle *x, char *filename, short path, short feature_to_export)
{
    long err;
    t_filehandle fh;
    err = path_createsysfile(filename, path, 'TEXT', &fh);
    switch (feature_to_export)
    {
        case 0:
        {
            t_ptr_size length_alphabet = getAlphabet(x);
            if (length_alphabet == -1)
            {
                object_post((t_object *)x, FILE_WRITE_ERROR);
                break;
            }
            t_ptr_size len, buff_size = sizeof(long) + sizeof(char);
            char *buff = sysmem_newptr(buff_size);
            for (long i = 0; i < length_alphabet; i++)
            {
                len = snprintf(buff, buff_size, "%ld%c", x->alphabet[i], ' ');
                err = sysfile_write(fh, &len, buff);
                if (err != 0)
                {
                    object_post((t_object *)x, FILE_WRITE_ERROR);
                    break;
                }
            }
            sysmem_freeptr(x->alphabet);
            break;
        }
        case 1:
        {
            t_ptr_size len, buff_size = sizeof(long) + sizeof(char);
            char *buff = sysmem_newptr(buff_size);
            if (buff == NULL)
            {
                object_post((t_object *)x, MEMORY_ALLOCATION_ERROR);
                break;
            }
            for (long i = 0; i < x->input_index; i++)
            {
                len = snprintf(buff, buff_size, "%ld%c", x->states[i].transitionElement, ' ');
                err = sysfile_write(fh, &len, buff);
                if (err != 0)
                {
                    object_post((t_object *)x, FILE_WRITE_ERROR);
                    break;
                }
            }
            sysmem_freeptr(buff);
            break;
        }
        case 2:
        {
            t_ptr_size len, buff_size = sizeof(long) + sizeof(char);
            char *buff = sysmem_newptr(buff_size);
            if (buff == NULL)
            {
                object_post((t_object *)x, MEMORY_ALLOCATION_ERROR);
                break;
            }
            for (long i = 0; i < x->output_index; i++)
            {
                len = snprintf(buff, buff_size, "%ld%c", x->output_string[i], ' ');
                err = sysfile_write(fh, &len, buff);
                if (err != 0)
                {
                    object_post((t_object *)x, FILE_WRITE_ERROR);
                    break;
                }
            }
            sysmem_freeptr(buff);
            break;
        }
        case 3:
        {
            t_ptr_size json_length = json(x);
            if (json_length == -1)
            {
                object_post((t_object *)x, FILE_WRITE_ERROR);
                break;
            }
            else
            {
                err = sysfile_write(fh, &json_length, x->json);
                sysmem_freeptr(x->json);
                if (err != 0)
                {
                    object_post((t_object *)x, FILE_WRITE_ERROR);
                    break;
                }
            }
            break;
        }
    }
    sysfile_close(fh);
}




void factorOracle_dowrite(t_factorOracle *x, t_symbol *s, short argc, t_atom *argv)
{
    t_fourcc filetype = 'TEXT', outtype;
    char filename[512];
    short path;
    if (s == gensym(""))
    {
        if (saveasdialog_extended(filename, &path, &outtype, &filetype, 1))
        {
            return;
        }
    }
    else
    {
        strcpy(filename, s->s_name);
        path = path_getdefault();
    }
    factorOracle_writefile(x, filename, path, (short*)atom_getlong(argv+0));
}




void factorOracle_writealphabet(t_factorOracle *x, t_symbol *s)
{
    critical_enter(0);
    if (x->input_index < 1)
    {
        object_post((t_object *)x, EMPTY_ORACLE_ERROR);
    }
    else
    {
        t_atom *argv = (t_atom*)sysmem_newptr(sizeof(t_atom));
        atom_setlong(argv, 0);
        defer(x, (method)factorOracle_dowrite, s, 1, argv);
    }
    critical_exit(0);
}




void factorOracle_writeinput(t_factorOracle *x, t_symbol *s)
{
    critical_enter(0);
    if (x->input_index < 1)
    {
        object_post((t_object *)x, EMPTY_ORACLE_ERROR);
    }
    else
    {
        critical_enter(0);
        t_atom *argv = (t_atom*)sysmem_newptr(sizeof(t_atom));
        atom_setlong(argv, 1);
        defer(x, (method)factorOracle_dowrite, s, 1, argv);
    }
    critical_exit(0);
}




void factorOracle_writeoutput(t_factorOracle *x, t_symbol *s)
{
    critical_enter(0);
    if (x->output_index < 1)
    {
        object_post((t_object *)x, EMPTY_ORACLE_ERROR);
    }
    else
    {
        t_atom *argv = (t_atom*)sysmem_newptr(sizeof(t_atom));
        atom_setlong(argv, 2);
        defer(x, (method)factorOracle_dowrite, s, 1, argv);
    }
    critical_exit(0);
}




void factorOracle_writejson(t_factorOracle *x, t_symbol *s)
{
    critical_enter(0);
    if (x->input_index < 1)
    {
        object_post((t_object *)x, EMPTY_ORACLE_ERROR);
    }
    else
    {
        t_atom *argv = (t_atom*)sysmem_newptr(sizeof(t_atom));
        atom_setlong(argv, 3);
        defer(x, (method)factorOracle_dowrite, s, 1, argv);
    }
    critical_exit(0);
}




void factorOracle_openfile(t_factorOracle *x, char *filename, short path)
{
    t_filehandle fh;
    char *buffer;
    t_ptr_size size;
    if (path_opensysfile(filename, path, &fh, READ_PERM))
    {
        object_error((t_object *)x, "error opening %s", filename);
        return;
    }
    
    sysfile_geteof(fh, &size);
    buffer = sysmem_newptr(size + CHAR_BIT);
    sysfile_read(fh, &size, buffer);
    sysfile_close(fh);
    sprintf(buffer+size, "\n");
    
    t_atom *av;
    long ac = 0;
    t_max_err err = MAX_ERR_NONE;
    err = atom_setparse(&ac, &av, buffer);
    sysmem_freeptr(buffer);

    
    if (err != MAX_ERR_NONE)
    {
        // handle error
    }
    else
    {
        x->states = (t_state*)sysmem_resizeptr(x->states, (ac + x->input_limit) * sizeof(t_state));
        if (x->states == NULL)
        {
            object_error((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
            return;
        }
        x->input_limit += ac;
        
        for (long i = 0; i < ac; i++)
        {
            if ((av+i)->a_type == A_LONG)
            {
                if (buildOracle(atom_getlong(av+i), x) != 0)
                {
                    object_post((t_object *)x, "%s", MEMORY_ALLOCATION_ERROR);
                    factorOracle_clear(x);
                    return;
                }
                else
                {
                    x->input_index += 1;
                }
            }
            else
            {
                // handle error
            }
        }
    }
    sysmem_freeptr(av);
}




void factorOracle_doread(t_factorOracle *x, t_symbol *s, short argc, t_atom *argv)
{
    t_fourcc filetype = 'TEXT', outtype;
    char filename[MAX_PATH_CHARS];
    short path;
    if (s == gensym(""))
    {
        if (open_dialog(filename, &path, &outtype, &filetype, 1))
        {
            return;
        }
    }
    else
    {
        strcpy(filename, s->s_name);
        path = path_getdefault();

        if (locatefile_extended(filename, &path, &outtype, &filetype, 1))
        {
            object_error((t_object *)x, "%s: not found", s->s_name);
            return;
        }
    }
    factorOracle_openfile(x, filename, path);
}




void factorOracle_read(t_factorOracle *x, t_symbol *s)
{
    critical_enter(0);
    defer(x, (method)factorOracle_doread, s, 0, NULL);
    critical_exit(0);
}




void factorOracle_mode(t_factorOracle *x, long option)
{
    if (option < 0) {
        x->mode = 0;
    } else if (option > 1) {
        x->mode = 1;
    } else {
        x->mode = option;
    }
}




void factorOracle_probability(t_factorOracle *x, double probability)
{
    if (probability > 1.0) {
        x->probability = 1.0;
    } else if (probability < 0.0) {
        x->probability = 0;
    } else {
        x->probability = probability;
    }
}




long jumpBack(t_factorOracle *x, long stateIndex)
{
    long linkIndex;
    while (x->states[stateIndex].suffixLink != 0)
    {
        linkIndex = x->states[stateIndex].suffixLink;
        if ((stateIndex - linkIndex) > 1) {
            return linkIndex;
        } else {
            stateIndex = linkIndex;
        }
    }
    return 0;
}




long mode_1(t_factorOracle *x) {
    return factorOracle_walk(x);
}

long mode_2(t_factorOracle *x) {
    return 0;
}

long mode_3(t_factorOracle *x) {
    return 0;
}




long factorOracle_walk(t_factorOracle *x)
{
    if ((x->output_state == -1) || (x->output_state == x->input_index))
    {
        long suffixState = jumpBack(x, x->input_index);
        x->output_state = suffixState + 1;         return x->states[suffixState].transitionElement;
    }
    
    double n = (double)rand() / (double)((unsigned)RAND_MAX + 1);
    
    if ((n >= x->probability) && (x->states[x->output_state].suffixLink != 0))
    {
        long suffixState = x->states[x->output_state].suffixLink;
        x->output_state = suffixState + 1;
        
        return x->states[suffixState].transitionElement;
    }
    else
    {
        double nn = n * x->states[x->output_state].numberOfTransitionElements;
        for (long i = 0; i <= x->states[x->output_state].numberOfTransitionElements; i++)
        {
            if (((double)i <= nn) && (nn < ((double)i + 1.0)))
            {
                x->output_state = x->states[x->output_state].transitionEndStates[i];
                break;
            }
        }
        return x->states[x->output_state - 1].transitionElement;
    }
}
