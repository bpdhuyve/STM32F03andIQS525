:://---------- set tool parameters that dont change ----------
@echo off
set tool_path=C:\depot\Software\uc_dsp_platform_2\base\304\main\draw\image convertor\tool\lcd-image-converter.exe
set preset_name=psistyle_image_rgb16_R5G6B5
set template_file=C:\depot\Software\uc_dsp_platform_2\base\304\main\draw\image convertor\templates\StdDraw_template_image_rgb16_R5G6B5.tmpl
set working_folder=C:\depot\Software\uc_dsp_platform_2\base\304\main\draw\image convertor\example

:://---------- put images you want to convert here without .png extension ----------
call:function_convert_image test
goto:eof


:://------------------------------------------------------------------------------------------------//
:: convert function 
:://------------------------------------------------------------------------------------------------//
:function_convert_image
echo converting %1.png

:: set tool parameters that that change per file
set input_filename_without_png_extention=%1

:: assemble final tool params
set doc_name=%input_filename_without_png_extention%
set input_file=%working_folder%\%input_filename_without_png_extention%.png
set output_file=%working_folder%\image_%input_filename_without_png_extention%.h

:: execute tool
"%tool_path%" --mode=convert-image --doc-name="%doc_name%" --preset-name="%preset_name%" --input="%input_file%" --output="%output_file%" --template="%template_file%"
:://------------------------------------------------------------------------------------------------//