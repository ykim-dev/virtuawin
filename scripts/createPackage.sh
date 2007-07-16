#
# Script for automating the build and release process as much as possible.
#
# Preconditions:
# - shell already configured to run gcc with MinGW and CVS (i.e. CVSROOT and CVS_RSH set)
# - A helpfile compiler (set HELPCOMPILER)
# - Inno Setup (set SETUPCOMPILER if not in standard install location)
# - emacs (or set EDITOR to another good text editor)
# - zip (set ZIP) or winzip with commandline extension
# - ncftp (if uploading to SF)
#
# Variables
# MSYS need windows style /? arguments to be double slashes, i.e. //? use a varable to handle this.
SLASH="/"
if [ `uname | sed -e "s/^MINGW.*/MINGW/"` == 'MINGW' ] ; then
    SLASH="//"
fi
if [ -z "$EDITOR" ] ; then
    EDITOR=emacs
fi
if [ -z "$HELPCOMPILER" ] ; then
    HELPCOMPILER="/cygdrive/c/helpcompiler/HCW"
fi
if [ -z "$SETUPCOMPILER" ] ; then
    SETUPCOMPILER="/cygdrive/c/Program Files/Inno Setup 5/Compil32"
fi
if [ -z "$WINZIP" ] ; then
    WINZIP="/cygdrive/c/Program\ Files/WinZip/wzzip"
fi

if [ -z "$1" ] ; then
    echo "Usage: createPackage <version> (e.g 3.X.x)"
    exit -1
fi

version=$1
file_ver=`echo $1 | sed s/' '/_/g`

echo Creating VirtuaWin package $version - $file_ver

mkdir ./$file_ver
cd ./$file_ver

cvs checkout README.TXT
cvs update -d

mkdir tmp
mkdir tmp/standard
mkdir tmp/unicode

cat Defines.h | sed -c -e "s/vwVIRTUAWIN_NAME_VERSION _T(\"VirtuaWin v.*\")/vwVIRTUAWIN_NAME_VERSION _T(\"VirtuaWin v$version\")/" > Defines.h.tmp
mv Defines.h.tmp Defines.h
cat WinList/winlist.rc | sed -c -e "s/^CAPTION \"WinList v.*\"/CAPTION \"WinList v$version\"/" > WinList/winlist.rc.tmp
mv WinList/winlist.rc.tmp WinList/winlist.rc
cat Modules/Assigner/assigner.rc | sed -c -e "s/^CAPTION \"Assigner v.*\"/CAPTION \"Assigner v$version\"/" > Modules/Assigner/assigner.rc.tmp
mv Modules/Assigner/assigner.rc.tmp Modules/Assigner/assigner.rc
cat scripts/virtuawin.iss | sed -c -e "s/^AppVerName=VirtuaWin v.*/AppVerName=VirtuaWin v$version/" > scripts/virtuawin.iss.tmp
mv scripts/virtuawin.iss.tmp scripts/virtuawin.iss
$EDITOR HISTORY.TXT
cp Defines.h Messages.h Module/
read -p "Compile source? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo compiling helpfile
    cd Help
    $HELPCOMPILER -C -E virtuawin.hpj
    cd ..
    echo building standard
    ./build -S
    ./build
    cd WinList
    ./build -S
    ./build
    cd ..
    cd Modules/Assigner/
    make
    cd ../..
    echo copying standard
    cp ./VirtuaWin.exe ./tmp/standard/
    cp ./Icons/1[0-9].ico ./tmp/standard/
    cp ./Icons/20.ico ./tmp/standard/
    cp ./WinList/WinList.exe ./tmp/standard/
    cp ./Modules/Assigner/VWAssigner.exe ./tmp/standard/
    cp ./READMEII.TXT ./tmp/standard/README.TXT
    cp ./HISTORY.TXT ./tmp/standard/
    cp ./COPYING.TXT ./tmp/standard/
    cp ./userlist.cfg ./tmp/standard/
    cp ./tricky.cfg ./tmp/standard/
    cp ./Help/VirtuaWin.hlp ./tmp/standard/VirtuaWin.hlp
    cp ./scripts/virtuawin.iss ./tmp/standard/
    cp ./scripts/VirtuaWin5.0.ISL ./tmp/standard/
    echo done standard
    echo building unicode
    ./build -S
    ./build -u
    cd WinList
    ./build -S
    ./build -u
    cd ..
    echo copying unicode
    cp ./VirtuaWin.exe ./tmp/unicode/
    cp ./Icons/1[0-9].ico ./tmp/unicode/
    cp ./Icons/20.ico ./tmp/unicode/
    cp ./WinList/WinList.exe ./tmp/unicode/
    cp ./Modules/Assigner/VWAssigner.exe ./tmp/unicode/
    cp ./READMEII.TXT ./tmp/unicode/README.TXT
    cp ./HISTORY.TXT ./tmp/unicode/
    cp ./COPYING.TXT ./tmp/unicode/
    cp ./userlist.cfg ./tmp/unicode/
    cp ./tricky.cfg ./tmp/unicode/
    cp ./Help/VirtuaWin.hlp ./tmp/unicode/VirtuaWin.hlp
    cp ./scripts/virtuawin.iss ./tmp/unicode/
    cp ./scripts/VirtuaWin5.0.ISL ./tmp/unicode/
    cat tmp/unicode/virtuawin.iss | sed -e "s/^AppVerName=VirtuaWin/AppVerName=VirtuaWin Unicode/" > tmp/unicode/virtuawin.iss.tmp
    mv tmp/unicode/virtuawin.iss.tmp tmp/unicode/virtuawin.iss
    echo done unicode
fi

cd ./tmp/standard

read -p "Compile standard setup package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    "$SETUPCOMPILER" ${SLASH}cc virtuawin.iss
    echo done!
fi

cd ../unicode

read -p "Compile unicode setup package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    "$SETUPCOMPILER" ${SLASH}cc virtuawin.iss
    echo done!
fi

cd ../..

read -p "Assemble source package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo Creating source package
    rm -f VirtuaWin_source_$file_ver.zip
    if [ -z "$ZIP" ] ; then
        $WINZIP VirtuaWin_source_$file_ver.zip -P @./scripts/filelist
    else        
        $ZIP -9 -@ < ./scripts/filelist
        mv zip.zip VirtuaWin_source_$file_ver.zip
    fi
    echo Done!
fi

read -p "Assemble SDK package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo Creating SDK package
    rm -f VirtuaWin_SDK_$file_ver.zip
    cd Module
    if [ -z "$ZIP" ] ; then
        $WINZIP ../VirtuaWin_SDK_$file_ver.zip -P @../scripts/SDK_filelist
    else        
        $ZIP -9 -@ < ../scripts/SDK_filelist
        mv zip.zip ../VirtuaWin_SDK_$file_ver.zip
    fi
    cd ..
    echo Done!
fi

read -p "Move packages? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    mkdir ../Distribution >& /dev/null
    mv ./tmp/standard/output/setup.exe ../Distribution/VirtuaWin_setup_$file_ver.exe
    mv ./tmp/unicode/output/setup.exe ../Distribution/VirtuaWin_unicode_setup_$file_ver.exe
    mv ./VirtuaWin_source_$file_ver.zip ../Distribution/
    mv ./VirtuaWin_SDK_$file_ver.zip ../Distribution/
fi

read -p "Commit changes? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo Committing changes with comment: Changed to V$version
    cvs commit -m "Changed to V$version"
fi

read -p "Label repository? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    export LABEL=`echo V$file_ver | sed s/'\.'/_/g`
    echo Labelling with: $LABEL
    cvs tag -R -F $LABEL
fi

read -p "Upload to SourceForge? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_setup_$file_ver.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_unicode_setup_$file_ver.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_source_$file_ver.zip
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_SDK_$file_ver.zip
    echo Done! Go to SourceForge and click Admin-Edit/Release Files-Add Release and then type $version and follow the instructions.
fi

read -p "Cleanup temporary files? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo cleanup temporary files
    rm -r ./tmp
    echo done!

    cd ..

    read -p "Delete $file_ver directory? [y/n] " -n 1
    echo
    if [ $REPLY == 'y' ] ; then
        rm -r ./$file_ver
    fi
fi

echo Packages created in "Distribution"!

