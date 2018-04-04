@echo off

set opts=-FC -GR- -EHa- -nologo -Zi
set code=%cd%
pushd build
cl %opts% %code%\pinky_crt_base.cpp -Fepinky
popd
