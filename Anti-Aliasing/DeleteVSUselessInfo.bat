@echo off  

attrib -h -r

setlocal enabledelayedexpansion    
  
for /r . %%a in (Debug) do (    
  if exist %%a (  
  echo "delete" %%a  
  rd /s /q "%%a"   
 )  
)  
  
for /r . %%a in (Release) do (    
  if exist %%a (  
  echo "delete" %%a  
  rd /s /q "%%a"   
 )  
)  
  
for /r . %%a in (ipch) do (    
  if exist %%a (  
  echo "delete" %%a  
  rd /s /q "%%a"   
 )  
)  
  
for /r . %%a in (*.sdf) do (    
  if exist %%a (  
  echo "delete" %%a  
  del "%%a"   
 )  
)  

for /r . %%a in (*.opensdf) do (    
  if exist %%a (  
  echo "delete" %%a  
  del "%%a"   
 )  
)

for /r . %%a in (*.v11.suo) do (    
  if exist %%a (  
  echo "delete" %%a  
  del "%%a"   
 )  
)    
  
for /r . %%a in (*.VC.db) do (    
  if exist %%a (  
  echo "delete" %%a  
  del "%%a"   
 )  
) 

for /r . %%a in (.vs) do (    
  if exist %%a (  
  echo "delete" %%a  
  rd /s /q "%%a"   
 )  
)  

for /r . %%a in (Events.log) do (    
  if exist %%a (  
  echo "delete" %%a  
  rd /s /q "%%a"   
 )  
)

for /r . %%a in (x64) do (    
  if exist %%a (  
  echo "delete" %%a  
  rd /s /q "%%a"   
 )  
)

pause 