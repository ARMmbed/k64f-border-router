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

// Supported RF shields
def radioshields = [
  "ATMEL",
  "MCR20"
  ]
  
def stepsForParallel = [:]

// Jenkins pipeline does not support map.each, we need to use oldschool for loop
for (int i = 0; i < targets.size(); i++) {
  for(int j = 0; j < toolchains.size(); j++) {
    for(int k = 0; k < radioshields.size(); k++) {
      def target = targets.get(i)
      def toolchain = toolchains.keySet().asList().get(j)
      def compilerLabel = toolchains.get(toolchain)
      def radioshield = radioshields.get(k)
      def stepName = "${target} ${toolchain} ${radioshield}"
      stepsForParallel[stepName] = buildStep(target, compilerLabel, toolchain, radioshield)
    }
  }
}

timestamps {
  parallel stepsForParallel
}

def buildStep(target, compilerLabel, toolchain, radioshield) {
  return {
    stage ("${target}_${compilerLabel}_${radioshield}") {
      node ("${compilerLabel}") {
        deleteDir()
        dir("k64f-border-router") {
          checkout scm

          if ("${radioshield}" == "MCR20") {
            // Replace default rf shield
            execute("sed -i 's/\"value\": \"ATMEL\"/\"value\": \"MCR20\"/' mbed_app.json")
          }
  
          execute("mbed deploy --protocol ssh")
          //Checkout mbed-os master
          dir("mbed-os") {
            execute("git fetch origin master")
            execute("git checkout FETCH_HEAD")
          }
          execute("mbed compile --build out/${target}_${compilerLabel}_${radioshield}/ -m ${target} -t ${toolchain} -c")
        }
        archive '**/k64f-border-router.bin'
      }
    }
  }
}
