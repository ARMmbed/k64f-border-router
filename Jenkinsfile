properties ([[$class: 'ParametersDefinitionProperty', parameterDefinitions: [
  [$class: 'StringParameterDefinition', name: 'mbed_os_revision', defaultValue: 'master', description: 'Revision of mbed-os to build'],
  [$class: 'BooleanParameterDefinition', name: 'smoke_test', defaultValue: false, description: 'Enable to run HW smoke test after building']
  ]]])

try {
  echo "Verifying build with mbed-os version ${mbed_os_revision}"
  env.MBED_OS_REVISION = "${mbed_os_revision}"
} catch (err) {
  def mbed_os_revision = "master"
  echo "Verifying build with mbed-os version ${mbed_os_revision}"
  env.MBED_OS_REVISION = "${mbed_os_revision}"
}

try {
  echo "Run smoke tests: ${smoke_test}"
} catch (err) {
  def smoke_test = "false"
  echo "Run smoke tests: ${smoke_test}"
}

// Map test suites to corresponding RaaS instances
def raas = [
  "lowpan_k64f_border_router_smoke_atmel.json": "8001",
  "lowpan_k64f_border_router_smoke_mcr20.json": "8034"
  ]

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

def parallelRunSmoke = [:]

// Need to compare boolean against string value
if ( smoke_test == "true" ) {
  // Generate smoke tests based on suite amount
  for(int i = 0; i < raas.size(); i++) {
    def suite_to_run = raas.keySet().asList().get(i)
    def raasPort = raas.get(suite_to_run)
    // Parallel execution needs unique step names. Remove .json file ending.
    def smokeStep = "${raasPort} ${suite_to_run.substring(0, suite_to_run.indexOf('.'))}"
    parallelRunSmoke[smokeStep] = run_smoke(targets, toolchains, radioshields, raasPort, suite_to_run)
  }
}

timestamps {
  parallel stepsForParallel
  parallel parallelRunSmoke
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

          // Set mbed-os to revision received as parameter
          execute("mbed deploy --protocol ssh")
          dir("mbed-os") {
            execute ("git checkout ${env.MBED_OS_REVISION}")
          }
          execute("mbed compile --build out/${target}_${compilerLabel}_${radioshield}/ -m ${target} -t ${toolchain} -c")
        }
        stash name: "${target}_${compilerLabel}_${radioshield}", includes: '**/k64f-border-router.bin'
        archive '**/k64f-border-router.bin'
        step([$class: 'WsCleanup'])
      }
    }
  }
}

def run_smoke(targets, toolchains, radioshields, raasPort, suite_to_run) {
  return {
    // Remove .json from suite name
    def suiteName = suite_to_run.substring(0, suite_to_run.indexOf('.'))
    stage ("smoke_${raasPort}_${suiteName}") {
      node ("linux_test") {
        deleteDir()
        dir("mbed-clitest") {
          git "git@github.com:ARMmbed/mbed-clitest.git"
          execute("git checkout ${env.LATEST_CLITEST_REL}")
          execute("git submodule update --init --recursive testcases")

          dir("testcases") {
            execute("git checkout master")
            dir("6lowpan") {
              execute("git checkout master")
            }
          }

          for (int i = 0; i < targets.size(); i++) {
            for(int j = 0; j < toolchains.size(); j++) {
              for(int k = 0; k < radioshields.size(); k++) {
                def target = targets.get(i)
                def toolchain = toolchains.keySet().asList().get(j)
                def compilerLabel = toolchains.get(toolchain)
                def radioshield = radioshields.get(k)
                unstash "${target}_${compilerLabel}_${radioshield}"
              }
            }
          }
          env.RAAS_USERNAME = "user"
          env.RAAS_PASSWORD = "user"
          execute("python clitest.py --suitedir testcases/suites/ --suite ${suite_to_run} --type hardware --reset --raas 193.208.80.31:${raasPort} --failure_return_value -vvv -w --log log_${raasPort}_${suiteName}")
          archive "log_${raasPort}_${suiteName}/**/*"
        }
      }
    }
  }
}
