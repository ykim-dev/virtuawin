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

if [ $1 == "" ] ; then 
    echo "Usage: createPackage <version> (e.g 3.X.x)"
    exit -1
fi

echo Creating VirtuaWin package $1

mkdir ./$1
cd ./$1

cvs checkout .

read -p "Label repository? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    export LABEL=`echo V$1 | sed s/'\.'/_/g`
    echo Labelling with: $LABEL
    cvs tag -R -F $LABEL
fi

mkdir tmp
mkdir tmp/standard
mkdir tmp/unicode

$EDITOR Defines.h
read -p "Compile source? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo compiling helpfile
    cd Help
    $HELPCOMPILER /C /E virtuawin.hpj
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
    echo done unicode
fi

cd ./tmp/standard

$EDITOR virtuawin.iss

read -p "Compile standard setup package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    "$SETUPCOMPILER" /cc virtuawin.iss
    echo done!
fi

cd ../unicode

$EDITOR virtuawin.iss

read -p "Compile unicode setup package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    "$SETUPCOMPILER" /cc virtuawin.iss
    echo done!
fi

cd ../..

read -p "Assemble source package? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo Creating source package
    rm -f VirtuaWin_source_$1.zip
    if [ -z "$ZIP" ] ; then
        $WINZIP VirtuaWin_source_$1.zip -P @./scripts/filelist
    else        
        $ZIP -@ < ./scripts/filelist
        mv zip.zip VirtuaWin_source_$1.zip
    fi
    echo Done!
fi

read -p "Move packages? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    mkdir ../Distribution >& /dev/null
    mv ./tmp/standard/output/setup.exe ../Distribution/VirtuaWin_setup_$1.exe
    mv ./tmp/unicode/output/setup.exe ../Distribution/VirtuaWin_unicode_setup_$1.exe
    mv ./VirtuaWin_source_$1.zip ../Distribution/
fi

read -p "Upload to SourceForge? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_setup_$1.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_unicode_setup_$1.exe
    ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se upload.sourceforge.net /incoming ../Distribution/VirtuaWin_source_$1.zip
    echo Done! Go to SourceForge and click Admin-Edit/Release Files-Add Release and then type $1 and follow the instructions.
fi

read -p "Cleanup temporary files? [y/n] " -n 1
echo
if [ $REPLY == 'y' ] ; then
    echo cleanup temporary files
    rm -r ./tmp
    echo done!

    cd ..

    read -p "Delete $1 directory? [y/n] " -n 1
    echo
    if [ $REPLY == 'y' ] ; then
        rm -r ./$1
    fi
fi

echo Packages created in "Distribution"!

