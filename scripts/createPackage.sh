#!tcsh
#
# Script for automating the build and release process as much as possible.
#
# Preconditions:
# - A file called .cvssettings in your home that sets the needed stuff for CVS access
# - A helpfile compiler
# - gcc with MinGW 
# - Inno Setup
# - emacs (or another good text editor)
# - ncftp
# - winzip with commandline extension
# - cvs 
#
# Variables
alias EDITOR        emacs
alias HELPCOMPILER  "/cygdrive/c/helpcompiler/HCW /C /E"
alias SETUPCOMPILER "/cygdrive/c/Program\ Files/Inno\ Setup\ 5/Compil32 /cc"
alias WINZIP        "/cygdrive/c/Program\ Files/WinZip/wzzip"

if ( $1 == "" ) then 
    echo "Usage: createPackage <version> (e.g 3.X.x)"
else 
    echo Creating VirtuaWin package $1

    mkdir ./$1
    cd ./$1
    source ~/.cvssettings

    cvs checkout .

    echo "Label repository? [y/n]"
    if ( $< == 'y' ) then
        setenv LABEL `echo V$1 | sed s/'\.'/_/g`
        echo Labling with: $LABEL
        cvs tag -R -F $LABEL
    endif

    EDITOR Defines.h
    echo "Start compilation? [y/n]"
    if ( $< == 'y' ) then
        make
        cd WinList
        make
        cd ..
        cd Modules/Assigner/
        make
        cd ../../..
        echo done compiling!
    endif

    mkdir tmp

    cp ./$1/VirtuaWin.exe ./tmp/
    cp ./$1/WinList/WinList.exe ./tmp/
    cp ./$1/Modules/Assigner/VWAssigner.exe ./tmp/
    cp ./$1/READMEII.TXT ./tmp/README.TXT
    cp ./$1/HISTORY.TXT ./tmp/
    cp ./$1/COPYING.TXT ./tmp/
    cp ./$1/userlist.cfg ./tmp/
    cp ./$1/tricky.cfg ./tmp/
    cp ./$1/Help/virtuawin.* ./tmp/
    cp ./$1/scripts/virtuawin.iss ./tmp/
    cp ./$1/scripts/VirtuaWin5.0.ISL ./tmp/
    cp ./$1/scripts/filelist ./tmp/
    
    echo compiling helpfile
    cd ./tmp/
    HELPCOMPILER virtuawin.hpj
    mv VIRTUAWIN.HLP VirtuaWin.hlp
    echo done!
    
    EDITOR virtuawin.iss

    echo "Compile setup package? [y/n]"
    if ( $< == 'y' ) then
        SETUPCOMPILER virtuawin.iss
        echo done!
    endif

    echo "Assemble source package? [y/n]"
    if ( $< == 'y' ) then
        echo Creating source package
        cd ../$1
        WINZIP source$1.zip -P @../tmp/filelist
        echo Done!
    endif

    cd ..

    echo "Move packages? [y/n]"
    if ( $< == 'y' ) then
        mkdir ./Distribution >& /dev/null
        mv ./tmp/output/setup.exe ./Distribution/vwsetup$1.exe
        mv ./$1/source$1.zip ./Distribution/
    endif

    echo "Upload to SourceForge? [y/n]"
    if ( $< == 'y' ) then
        ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se shell.sf.net /incoming ./Distribution/vwsetup$1.exe
        ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se shell.sf.net /incoming ./Distribution/source$1.zip
        echo Done! Go to SourceForge and click Admin-Edit/Release Files-Add Release and then type $1 and follow the instructions.
    endif

    echo cleanup temporary files
    rm -r ./tmp
    echo done!

    echo "Delete $1 directory? [y/n]"
    if  ( $< == 'y' ) then
        rm -r ./$1
    endif

    echo Packages created in "Distribution"!
endif

#
# $Log$
# Revision 1.5  2006/06/09 18:06:13  jopi
# Updated packaging scripts
#
# Revision 1.4  2006/04/05 15:10:31  jopi
# version info moved
#
# Revision 1.3  2005/03/15 07:09:30  jopi
# Minor update
#
# Revision 1.2  2005/03/15 06:58:22  jopi
# Minor update
#
#
