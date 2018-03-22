#!/bin/bash
bin_path=$1
output_path=$(dirname $bin_path)/Resources
renamed_path=$2

# list dependent libraries
libs=$(otool -L $bin_path | tail -n +2)

# split libs into a list
lib_array=(${libs// / })
for lib in "${lib_array[@]}"
do
    # make sure the path starts with the HOME folder. if not, that's system libs
    if [[ $lib == $HOME* ]] ;
    then
      # copy the lib
      cp $lib $output_path
      # change the search path in the source lib to point to the new path
      libname=$(basename $lib)
      install_name_tool $bin_path -change $lib $renamed_path/$libname

      # also change the search path of this library's dependencies
      copied_lib_path=$output_path/$libname
      deps=$(otool -L $copied_lib_path | tail -n +2)
      deps_array=(${deps// / })
      for dep in "${deps_array[@]}"
      do
        depname=$(basename $dep)
        install_name_tool $copied_lib_path -id $renamed_path/$depname
        if [[ $dep == $HOME* ]] ;
        then
          install_name_tool $copied_lib_path -change $dep $renamed_path/$depname
        fi
      done
    fi
done
