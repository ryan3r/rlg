# Remove the releases
Write-Host "Removing releases"
plink ryan@zoom.pi 'rm -f ~/cs327-releases/*'

# Move the build to the desktop
Write-Host "Moving tar ball to desktop"
move *.tar.gz c:\users\ryan3\Desktop
