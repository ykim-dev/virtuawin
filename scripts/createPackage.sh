#!tcsh
  
if ( $1 == "" ) then 
    echo "Usage: createPackage <version> (e.g 2.X.x)"
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

    emacs VirtuaWin.h VirtuaWin.rc
    echo "Start compilation? [y/n]"
    if ( $< == 'y' ) then
        make
        cd WinList
        make
        cd ../..
        echo done compiling!
    endif

    mkdir tmp

    cp ./$1/VirtuaWin.exe ./tmp/
    cp ./$1/WinList/WinList.exe ./tmp/
    cp ./$1/READMEII.TXT ./tmp/README.TXT
    cp ./$1/HISTORY.TXT ./tmp/
    cp ./$1/COPYING.TXT ./tmp/
    cp ./$1/userlist.cfg ./tmp/
    cp ./$1/tricky.cfg ./tmp/
    cp ./$1/Help/virtuawin.* ./tmp/
    cp ./virtuawin.iss ./tmp/
    cp ./VirtuaWin5.0.ISL ./tmp/
    
    echo compiling helpfile
    cd ./tmp/
    /cygdrive/c/Program\ Files/Help\ Workshop/HCW /C /E virtuawin.hpj
    mv VIRTUAWIN.HLP VirtuaWin.hlp
    echo done!
    
    emacs virtuawin.iss

    echo "Compile setup package? [y/n]"
    if ( $< == 'y' ) then
        /cygdrive/c/Program\ Files/Inno\ Setup\ 5/Compil32 /cc virtuawin.iss
        echo done!
    endif

    echo "Assemble source package? [y/n]"
    if ( $< == 'y' ) then
        echo Creating source package
        cd ../$1
        /cygdrive/c/Program\ Files/WinZip/wzzip source$1.zip -P @../filelist
        echo Done!
    endif

    cd ..

    echo "Move packages? [y/n]"
    if ( $< == 'y' ) then
        mv ./tmp/output/setup.exe ./Distribution/vwsetup$1.exe
        mv ./$1/source$1.zip ./Distribution/
    endif

    echo "Upload to SourceForge? [y/n]"
    if ( $< == 'y' ) then
        ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se 66.35.250.221 /incoming ./Distribution/vwsetup$1.exe
        ncftpput -d ./ftpsession.log -u anonymous -p virtuawin@home.se 66.35.250.221 /incoming ./Distribution/source$1.zip
        echo Done! Go to SourceForge and click Admin-Edit/Release Files-Add Release and then type $1 and follow the instructions.
    endif

    echo cleanup temporary files
    rm -r ./tmp
    echo done!

    echo "Delete $1 directory? [y/n]"
    if  ( $< == 'y' ) then
        rm -r ./$1
    endif

    echo Packages created!
endif
