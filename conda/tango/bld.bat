echo "... [tango/bld.bat] enter"
cd tango

echo "... [tango/bld.bat] cmake compile"
cmake -Bbuild -H. -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -DPYTHON_SITE_PACKAGES_DIR=%SP_DIR% -DCMAKE_FIND_ROOT_PATH=%LIBRARY_PREFIX%
IF %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo "... [tango/bld.bat] cmake build"
cmake --build build --config Release --target install
IF %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo "... [tango/bld.bat] exit"
