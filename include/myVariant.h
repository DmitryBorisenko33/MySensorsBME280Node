//дефолтная декларация пинов находится в ядре
//по этому пути: .platformio\packages\framework-arduinonordicnrf5\variants\nRF52DK
//в файлах variant.cpp и variant.h
//в variant.cpp находится массив g_ADigitalPinMap[] в этом массиве пины которые будет назначать ядро
//следует менять пины в этом файле но когда вы обновите ядро следует поменять менять заново
