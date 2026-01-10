# Exit when any command fails so no further data is corrupted
set -e
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
trap 'if [ "$?" -ne "0" ]; then echo "\"${last_command}\" command failed with exit code $?."; fi' EXIT

echo 'Packaging GTK installation for pipeline usage. This may take a while...'

# Store current directory
CUR_DIR=`pwd`

# Go to gtk directory
cd ~/gtk
if [[ -d "inst.bak" ]]; then
  echo 'There is already a directory called \"\$HOME/gtk/inst.bak\".'
  echo 'Aborting to not corrupt jhbuild environment! Clean up manually!'
  exit 1
fi

# Move jhbuild environment so we can copy required data
mv inst inst.bak

# Create fake installation directory as target for copy operations
mkdir inst

cd inst

# Copy CMake
mkdir bin
# cp ../inst.bak/bin/cmake bin/cmake
# mkdir -p share/cmake-3.20
# cp -r ../inst.bak/share/cmake-3.20 share/

# Copy CTest
cp ../inst.bak/bin/ctest bin/ctest

# Copy Pkg-Config 
cp ../inst.bak/bin/pkgconf bin/pkgconf

# Copy GetText
cp ../inst.bak/bin/gettext bin/gettext
cp ../inst.bak/bin/gettext.sh bin/gettext.sh
cp ../inst.bak/bin/xgettext bin/xgettext
cp ../inst.bak/bin/msgmerge bin/msgmerge
cp ../inst.bak/bin/msgfmt bin/msgfmt
cp ../inst.bak/bin/msgcat bin/msgcat
cp -r ../inst.bak/share/gettext share/gettext
cp -r ../inst.bak/share/gettext-0.21 share/gettext-0.21

# Copy Libraries
# (do not do this for specific libraries to circumvent issues with new dependencies)
cp -r ../inst.bak/lib .
cp -r ../inst.bak/include .

#Copy files required by bundler
mkdir -p share/glib-2.0
cp -r ../inst.bak/share/glib-2.0/schemas share/glib-2.0/
cp -r ../inst.bak/share/locale share
cp -r ../inst.bak/share/themes share
cp -r ../inst.bak/share/icons share
cp ../inst.bak/bin/gdk-pixbuf-query-loaders bin/
cp ../inst.bak/bin/gtk-query-immodules-3.0 bin/

# Remove old packaged data if it exists
rm -f $CUR_DIR/gtk-bin.tar.gz

# Package directory
cd ..
mkdir gtk
mv inst gtk/
tar -zcf $CUR_DIR/gtk-bin.tar.gz gtk

# Undo copy operation so we have a running jhbuild environment again
rm -rf gtk
mv inst.bak inst
