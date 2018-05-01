mkdir build_win > $null 2> $null
cd build_win

cmake ..
if(!$?) { cd ..; exit 1 }

cmake --build .
if(!$?) { cd ..; exit 1 }

cd ..
mkdir out > $null 2> $null
cp build_win/Debug/rlg.exe out
mkdir out/defs > $null 2> $null
cp defs/monster_desc.txt out/defs
cp defs/object_desc.txt out/defs
