#!/bin/bash

# this scriot uses "cproto" to extract functions prototypes from bacnet source files
# into 3 different files:
#    typedefs   - defines typedefs from BACnet syack functions as bacnet_func_<function name>
#    functions  - defines class members using above typedefs to be included in the BACnet dynamic library class
#    loading    - C++ code to be inlcuded in the class memeber to load dynamiclly BACnet stack functions using dslym()
#
#    Note: these files may require manual editiong before they could be used
#

files=$(ls *.c)
#files=iam.c

rm typedefs functions loading

for file in $files
do
    cproto -I ../include/ -I ../ports/linux $file | gawk  'BEGIN {n=1;}
            NR>1 { 
            gsub("_Bool","bool");
            str=$0;
            match(str, "(\\w+)[(]", a);
            print "bacnet_func_"a[1], a[1]"{nullptr};" >> "functions";
            funcs[n++] = a[1];
            print "typedef", gensub("(\\w+)[(]", "(*bacnet_func_\\1)(", 1)  >> "typedefs";
            }
            END {
             for(i=1; i<n; i++) {
                  print funcs[i]" = (bacnet_func_"funcs[i]")dlsym(bacnetHandle,\""funcs[i]"\");" >> "loading";
                  print "if(!"funcs[i] ") {\n loadError=true; \ncerr << \"ERROR: Cannot load " funcs[i]"\" << endl; \n}\n" >> "loading";
              }
            }
            '
done

   
     
