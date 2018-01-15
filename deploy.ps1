	param(
	[Parameter(Mandatory=$true)][int]$assignment,
	[switch]$git = $false
)

# Push the repo
if($git) {
	Write-Host "Pushing changes"
	git push 1> $null 2> $null
}

# Start the vm
if(-Not ("$(vagrant status)" -Match "running")) {
	Write-Host "Starting dev vm"
	vagrant up 1> $null 2> $null
}

# Build the tar ball
Write-Host "Building tar ball"
vagrant ssh -c "/workspace/deploy $assignment" 1> $null 2> $null

# Uploading tar ball
Write-Host "Uploading tar ball"
pscp -q Ray_Ryan.assignment-$assignment.tar.gz ryan@zoom.pi:/home/ryan/cs327-releases

if(!$?) {
	exit 1
}

# Update the latest link
plink ryan@zoom.pi "ln -s /home/ryan/cs327-releases/Ray_Ryan.assignment-$assignment.tar.gz /home/ryan/cs327-releases/latest"
