#!/bin/bash

SCHNELL=/schnell
API=paEr2RZGNJjA
STUDIO=http://chevre.hwlab.suse.de

SLE11=$SCHNELL/CD-ARCHIVE/SLE11
SLES11GA=$SLE11/SLES-11-GM
SLES11SP1=$SLE11/SLES-11-SP1-GM
SLES11SP2=$SLE11/SLES-11-SP2-GM
SLED11GA=SLE11/SLED-11-GM
SLED11SP1=$SLE11/SLED-11-SP1-GM
SLED11SP2=$SLE11/SLED-11-SP2-GM
RPMDIR=./rpms

if [ "$1" != "" ]; then

  cd $1

fi

mkdir -p $RPMDIR
rm clone-result 

# Get the hostname

SERVERHOST=$(grep "HOSTNAME=" env.txt | head -1 | sed "s/HOSTNAME=//g")

#Get the arch

tmpcpu=$(grep -m 1 CPU= env.txt)
tmpcpu=${tmpcpu#*=}
BLA=$tmpcpu

[ $BLA = "i386" -o $BLA = "i586" -o $BLA = "i686" ] && ARCH=i586
[ $BLA = "x86_64" ] && ARCH=x86_64

#Figure out which distro it is

grep -qc "SUSE Linux Enterprise" basic-environment.txt

if [ $? -eq 1 ]; then
  
  echo "FAILED/Unsupported distribution"
  exit

fi

grep -qc "novell-release" basic-environment.txt

if [ $? -eq 0 ]; then
  
  echo "FAILED/OES is currently not supported"
  exit

fi

grep -qc "slepos-release" basic-environment.txt

if [ $? -eq 0 ]; then

  echo "FAILED/SLEPOS is currently not supported"
  exit

fi

grep -qc "Server 11" basic-environment.txt && SUSEVER=SLES11
grep -qc "Desktop 11" basic-environment.txt && SUSEVER=SLED11
grep -qc "Server 10" basic-environment.txt && SUSEVER=SLES10
grep -qc "Desktop 10" basic-environment.txt && SUSEVER=SLED10

TMPVER=$(grep PATCHLEVEL basic-environment.txt)

spver=${TMPVER/ /}
spver=${spver/ /}
eval $spver

INDEXURL=http://kueue.hwlab.suse.de/index/$SUSEVER-SP$PATCHLEVEL-GM-$ARCH-INDEX.gz
INDEXURL=${INDEXURL/586/386} # annoying

SDKINDEXURL=http://kueue.hwlab.suse.de/index/$SUSEVER-SP$PATCHLEVEL-SDK-GM-$ARCH-INDEX.gz
SDKINDEXURL=${SDKINDEXURL/586/386} # annoying

curl -f -s -o SDKINDEX.gz $SDKINDEXURL
curl -f -s -o INDEX.gz $INDEXURL

if [ $? -ne 0 ]; then

  echo "FAILED/The version of operating system in this supportconfig is not supported"
  exit

fi

if ! test -e $INDEX; then

  echo "FAILED/Couldn't find the index file."
  exit

fi

cat > clone.spec << END

%define	_rpmdir .
Name:           clone
License:        GPL v2 or later; LGPL v2.1 or later
Group:          System/Base
AutoReqProv:    off
Version:        1
Release:        1
Summary:        dummy package for cloning a system through requirements
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
END

awk '/==============================/{n++}{print > n "RPM.txt" }' rpm.txt
TMPFILE=$(grep -l "# rpm -qa --queryformat" *RPM*)
awk 'FNR>3' $TMPFILE > clone-rpm.txt
TOTAL=$(wc -l clone-rpm.txt | awk '{print $1}')
PROG=0

echo "TOTAL" $TOTAL

while read line; do

let PROG=PROG+1
echo "PROG" $PROG 

  if [ -n "$line" ]; then

    # get package details

    pkg=$(echo $line | awk '{print $1}')
    pkgver=$(echo $line | awk '{print $NF}')
    vendor=$(echo $line | sed "s/$pkg //g" | sed "s/ $pkgver//g")

    # ignore gpg-pubkey

    if [ $pkg = "gpg-pubkey" ]; then

        continue

    fi

    # ignore supportutils

    if [[ "$vendor" == *NTS* ]]; then

        continue

    fi

    # ignore NAME PACKAGE VERSION

    if [ "$pkg" = "NAME" ] && [ "$pkgver" = "VERSION" ]; then

        continue

    fi

    # check if its a PTF, and if so, download it

    if [[ "$pkgver" =~ .*PTF.* ]]; then

        ptfurl=$(curl -A "clone.sh" -s http://kueue.hwlab.suse.de:8080/ptf/$pkg-$pkgver.$ARCH.rpm)
        
        if [[ $ptfurl == http* ]]; then

            curl -s -o $RPMDIR/$pkg-$pkgver.$ARCH.rpm $ptfurl
            echo Requires: $pkg = $pkgver >> clone.spec
            continue

        fi

    fi
                            
    # lookup package in INDEX.gz

    if [[ "$pkg" == *32bit* ]]; then 

        INDEXPKG=$(zgrep /$pkg-[0-9] INDEX.gz|tr "/" "\n"|tail -1)

        if [ -z "$INDEXPKG" ]; then

        INDEXPKG=$(zgrep /$pkg-[0-9] SDKINDEX.gz|tr "/" "\n"|tail -1)

        fi

    else

        INDEXPKG=$(zgrep /$pkg-[0-9] INDEX.gz|grep -v 32bit|tr "/" "\n"|tail -1)

        if [ -z "$INDEXPKG" ]; then

        INDEXPKG=$(zgrep /$pkg-[0-9] SDKINDEX.gz|grep -v 32bit|tr "/" "\n"|tail -1)

        fi

    fi
          
    # if package not listed in index, log to clone-result

    if [ -z "$INDEXPKG" ]; then

        echo $pkg - $pkgver - $vendor >> clone-result

    # if package listed, compare version 

    else

        INDEXPKGVER=$(echo $INDEXPKG | sed "s/$pkg-//g" | sed "s/.$ARCH.rpm//g" | sed "s/.noarch.rpm//g" )
                                
        # if version matches, require package in .spec

        if [ "$INDEXPKGVER" == "$pkgver" ]; then

        echo Requires: $pkg = $pkgver >> clone.spec

        # if version doesn't match, check if the version is an official update
        # if it is, require in .spec, if it isn't, log to clone-result

        else

        VALID=$(curl -A "clone.sh" -s "http://kueue.hwlab.suse.de:8080/validversion/$SUSEVER-SP$PATCHLEVEL-$ARCH|$pkg|$pkgver")

        if [ "$VALID" == "1" ]; then
        
            echo Requires: $pkg = $pkgver >> clone.spec

        else

            echo $pkg - $pkgver - $vendor >> clone-result

        fi

      fi

    fi

  fi

done < clone-rpm.txt

cat >> clone.spec <<END
%description
Empty package that just "requires" all packages with exactly the versions and releases
that were installed on the original system, so it can be cloned exactly


Authors:
--------
    Anders Johansson <ajohansson@suse.com>


%prep

%build
%install
if [ ! -d %{buildroot}; then mkdir %{buildroot}; fi
touch %{buildroot}/foo
%files
/foo
%changelog
* Mon Jul 23 2012 ajohansson@suse.com
  Initial version
END

rpmbuild -bb --target $ARCH clone.spec &> /dev/null

if [ $? -eq 0 ]; then 
	
    mv $ARCH/clone* $RPMDIR
    echo "SUCCESS/$(echo $SUSEVER | sed 's/-//g')_SP$PATCHLEVEL/$ARCH/$SERVERHOST"

else
    
    echo "FAILED/rpmbuild failed."

fi    
