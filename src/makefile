CC=gcc
INCLUDE=-I core -I base_type -I builtin_object
CFLAGS=-Wall -ggdb3 -O0 -DWARF2 -DW_AT_call_file -DW_AT_call_line  ${INCLUDE}
LFLAGS=-lm

headers=\
v3_pool.h \
v3_string.h \
v3_tokenizer.h \
v3_error_object.h \
v3_eval.h \
v3_exception.h\
base_type/v3_base_type.h \
base_type/v3_null.h \
base_type/v3_undefined.h \
base_type/v3_bool.h \
base_type/v3_function_object.h \
base_type/v3_number_object.h \
base_type/v3_string_object.h \
base_type/v3_object.h \
base_type/v3_reference.h \
base_type/v3_conversion.h \
core/v3_core.h \
core/v3_dict.h \
core/v3_pool.h \
core/v3_vector.h \
core/v3_foundation.h \
core/v3_list.h \


objects=\
v3_tokenizer.o \
v3_parser.o \
v3js.o \
v3_eval.o \
v3_exception.o\
base_type/v3_function_object.o \
base_type/v3_number_object.o \
base_type/v3_object.o \
base_type/v3_string_object.o \
base_type/v3_null.o \
base_type/v3_undefined.o \
base_type/v3_bool.o \
base_type/v3_reference.o \
base_type/v3_conversion.o \
builtin_object/v3_Error.o \
builtin_object/v3_TypeError.o \
builtin_object/v3_Function.o \
builtin_object/v3_Number.o \
builtin_object/v3_Object.o \
core/v3_dict.o \
core/v3_foundation.o \
core/v3_list.o \
core/v3_vector.o \
core/v3_pool.o \


default:$(objects)
	$(CC) $(CFLAGS) -o v3js  $(objects) $(LFLAGS) 

$(objects):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LFLAGS)   

clean:
	rm -rf *.o

