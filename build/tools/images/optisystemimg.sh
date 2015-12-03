#!/bin/bash

function optiapk()
{
  7za x -o"$1" "$2"
#  find "$1/res/" -name "*.png" |xargs optipng -o5
# rm assests and res/raw: these folders must not be compressed
#  mv $2 $2.zip
#  7za d "$2.zip" "lib/*"
#  mv $2.zip $2
  rm -rf $1/assets/
  rm -rf $1/res/raw*/
#  rm -rf $1/lib/
  7za a -tzip "$2" "$1/*" -mx9
  mv $2 $2u.apk
  out/host/linux-x86/bin/zipalign -f 4 "$2u.apk" "$2"
  rm -f $2u.apk
}

function optijar()
{
  7za x -o"$1" "$2"
#  rm -f $2
  7za a -tzip "$2" "$1/*" -mx9
  mv $2 $2u.apk
  out/host/linux-x86/bin/zipalign -f 4 "$2u.apk" "$2"
  rm -f $2u.apk
}

function optisystemimg()
{
 Tmp_folder="$1/../systemtmp"
 
 if [ -e $Tmp_folder ] ; then
  rm -rf $Tmp_folder
 fi
 mkdir $Tmp_folder
 F_app="$1/app"
# BF_app="$Tmp_folder/appbak"
 PF_app="$1/priv-app"
# BPF_app="$Tmp_folder/priv-appbak"

# cp -r $F_app $BF_app
# cp -r $PF_app $BPF_app
# cp ./mediatek/build/tools/images/optipng ./out/host/linux-x86/bin/
# chmod a+x ./out/host/linux-x86/bin/optipng

 for file_a in ${F_app}/*; do
  apk_file=`basename $file_a`
  cp $F_app/$apk_file $Tmp_folder/$apk_file.bak
  optiapk $Tmp_folder/$apk_file $F_app/$apk_file
 done

 for file_b in ${PF_app}/*; do
  apk_file2=`basename $file_b`
  cp $PF_app/$apk_file2 $Tmp_folder/$apk_file2.bak
  optiapk $Tmp_folder/$apk_file2 $PF_app/$apk_file2
 done

 FF_Folder="$1/framework"
 for file_c in ${FF_Folder}/*.apk; do
   apk_file3=`basename $file_c`
   cp $FF_Folder/$apk_file3 $Tmp_folder/$apk_file3.bak
   optiapk $Tmp_folder/$apk_file3 $FF_Folder/$apk_file3
 done

 for file_d in ${FF_Folder}/*.jar; do
  jar_file=`basename $file_d`
  cp $FF_Folder/$jar_file $Tmp_folder/$jar_file.bak
  optijar $Tmp_folder/$jar_file $FF_Folder/$jar_file
 done
}

img_file=$2
if [ ${img_file##*/} == "system.img" ] ; then
 echo "begin optimize system.img"
 optisystemimg $1
fi

