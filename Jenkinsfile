// List of targets to compile
def targets = [
  "K64F"
  ]
  
// Map toolchains to compilers
def toolchains = [
  ARM: "armcc",
  GCC_ARM: "arm-none-eabi-gcc",
  IAR: "iar_arm"
  ]
  
def stepsForParallel = [:]

// Jenkins pipeline does not support map.each, we need to use oldschool for loop
for (int i = 0; i < targets.size(); i++) {
  for(int j = 0; j < toolchains.size(); j++) {
    def target = targets.get(i)
    def toolchain = toolchains.keySet().asList().get(j)
    def compilerLabel = toolchains.get(toolchain)
    def stepName = "${target} ${toolchain}"
    stepsForParallel[stepName] = buildStep(target, compilerLabel, toolchain)
  }
}

timestamps {
  parallel stepsForParallel
}

def buildStep(target, compilerLabel, toolchain) {
  return {
    stage ("${target}_${compilerLabel}") {
      node ("${compilerLabel}") {
        deleteDir()
        dir("k64f-border-router") {
          checkout scm

          // Update target features to match newest mbed-os
          execute(sed -i 's/\"IPV6\", \"COMMON_PAL\"/\"NANOSTACK\", \"LOWPAN_BORDER_ROUTER\", \"COMMON_PAL\"/' mbed_app.json)
  
          execute ("mbed deploy --protocol ssh")
          //Checkout mbed-os master
          dir(mbed-os) {
            execute("git fetch origin master")
            execute("git checkout FETCH_HEAD")
          }
          execute("mbed compile --build out/${target}_${compilerLabel}/ -m ${target} -t ${toolchain} -c")
        }
        archive '**/k64f-border-router.bin'
      }
    }
  }
}
