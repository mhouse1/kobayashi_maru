*** Test Cases ***
hello_world on frdm_mcxn947_mcxn947_cpu0
    ${x}=                       Execute Command         include @${CURDIR}/hello_world.resc
    Create Terminal Tester      sysbus.flexcomm4lpuart4    timeout=5    defaultPauseEmulation=true

    Register Failing Uart String    ZEPHYR FATAL ERROR

    Wait For Line On Uart       *** Booting Zephyr OS build 9463d9a51d9c ***
    Wait For Line On Uart       Hello World! frdm_mcxn947