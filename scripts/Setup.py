
import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupPremake import PremakeConfiguration as PremakeRequirements
os.chdir('./../') # Change from devtools/scripts directory to root

premakeInstalled = PremakeRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if (premakeInstalled):
    if platform.system() == "Windows":
        print("\nRunning premake...")
        subprocess.call([os.path.abspath("./scripts/GenerateProjects.bat"), "nopause"])
    else:
        print("\nError: Supported platform is Windows only.")

    print("\nSetup completed!")
else:
    print("Premake is required to generate project files.")