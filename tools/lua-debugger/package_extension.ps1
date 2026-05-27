Write-Host "Compiling TypeScript..."
npx tsc

Write-Host "Packaging VS Code extension..."
vsce package --allow-missing-repository 

Write-Host "Done."