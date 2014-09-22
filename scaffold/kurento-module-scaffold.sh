 #!/bin/bash

expected_args=2

if [ $# -lt $expected_args ]
  then
    echo "Usage: kurento-module-scaffold <ModuleName> <output_directory> [opencv_filter]"
    exit
fi

currentDir=@CMAKE_INSTALL_PREFIX@/share/kurento/scaffold

camelName=$1
name=`echo $camelName | sed -e 's/\([A-Z]\)/\L\1/g'`
nameUndersScoreLower=`echo $camelName | sed -e 's/\([A-Z]\)/_\L\1/g' -e 's/^_//'`
nameScoreLower=`echo $camelName | sed -e 's/\([A-Z]\)/-\L\1/g' -e 's/^-//'`
nameUnderscoreUpper=`echo $nameUndersScoreLower | sed -e 's/\([a-z]\)/\U\1/g'`
outputDir=$2
openCVFilter=$3

#Change to output directory
mkdir -p $outputDir
cd $outputDir

#Create directory for module
mkdir $nameScoreLower
cd $nameScoreLower
git init

#Create folder tree
mkdir src
cd src
mkdir server

cat $currentDir/CMakeLists_server.txt.template > "CMakeLists.txt"
if [ -z "$openCVFilter" ]
  then
    mkdir gst-plugins
    cat $currentDir/CMakeLists_server_plugin.txt.template >> "CMakeLists.txt"
    cd gst-plugins

    cat $currentDir/CMakeLists_plugin.txt.template | sed -e "s/\${name}/$name/" -e "s/\${nameUnderscoreUpper}/$nameUnderscoreUpper/" > "CMakeLists.txt"

    cat $currentDir/plugin.c.template | sed -e "s/\${name}/$name/" -e "s/\${nameUndersScoreLower}/$nameUndersScoreLower/" > "$name.c"

    cat $currentDir/gstplugin.h.template | sed -e "s/\${nameUndersScoreLower}/$nameUndersScoreLower/g" -e "s/\${nameUnderscoreUpper}/$nameUnderscoreUpper/g" -e "s/\${camelName}/$camelName/g" > "gst$name.h"

    cat $currentDir/gstplugin.cpp.template | sed -e "s/\${name}/$name/" -e "s/\${nameUndersScoreLower}/$nameUndersScoreLower/" -e "s/\${nameUnderscoreUpper}/$nameUnderscoreUpper/" -e "s/\${camelName}/$camelName/" > "gst$name.cpp"

    cd ..
fi

cd server
mkdir implementation
mkdir interface
cd interface

cat $currentDir/filter.kmd.json.template | sed -e "s/\${name}/$name/" > "$name.kmd.json"

if [ -z "$openCVFilter" ]
  then
    cat $currentDir/filter.Filter.kmd.json.template | sed -e "s/\${name}/$name/" -e "s/\${camelName}/$camelName/" -e "s/\${extends}/Filter/"> "$name.$camelName.kmd.json"
else
    cat $currentDir/filter.Filter.kmd.json.template | sed -e "s/\${name}/$name/" -e "s/\${camelName}/$camelName/" -e "s/\${extends}/OpenCVFilter/"> "$name.$camelName.kmd.json"
fi

cd ..
cat $currentDir/CMakeLists_src.txt.template > "CMakeLists.txt"

cd ../..

cat $currentDir/config.h.cmake.template | sed -e "s/\${nameUnderscoreUpper}/$nameUnderscoreUpper/" > "config.h.cmake"
cat $currentDir/CMakeLists_root.txt.template | sed -e "s/\${nameScoreLower}/$nameScoreLower/" > "CMakeLists.txt"

#create debian directory
mkdir debian
cd debian
echo "9" > "compat"
touch copyright
touch docs

cat $currentDir/changelog.template | sed -e "s/\${nameScoreLower}/$nameScoreLower/" -e "s/\${date}/`date -R`/"> "changelog"
cat $currentDir/control.template | sed -e "s/\${nameScoreLower}/$nameScoreLower/" > "control"
cat $currentDir/rules.template > "rules"

if [ -z "$openCVFilter" ]
  then
    cat $currentDir/name.install.template > "$nameScoreLower.install"
else
  cat $currentDir/name.install.OpenCV.template > "$nameScoreLower.install"
fi

cat $currentDir/name-dev.install.template > "$nameScoreLower-dev.install"

mkdir source
cd source
echo "3.0 (native)" > "format"
#create .gitignore file
cd ../..
echo "build/" > ".gitignore"
echo "debian/$nameUndersScoreLower" >> ".gitignore"

git add CMakeLists.txt src .gitignore config.h.cmake debian
git commit -m "Initial commit"

echo "Folder tree created successfully"
