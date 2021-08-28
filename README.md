# FInalProject_RTOS.PADS
Proyecto Final de RTOS del Diplomado de SisElectrónicos PADS.IECA Equipo Alejandro García y César Mata

El objetivo de esta descripción es dar a conocer las especificaciones del programa finalProyect, que tiene como propósito implementar la funcionalidad de un reloj con alarma en la tarjeta FRDM-K64, y posteriormente, ser desplegado en una pantalla nokia 5110 LCD. El proyecto está realizado en base a un sistema operativo en tiempo real en FreeRTOS. 

Conexiones:
Debido a que la comunicación entre la tarjeta FRDM-K64 y la pantalla LCD es de tipo SPI, las conexiones que se tienen que hacer son las indicadas en la Figura 1 (anexado como imagen).


Archivos principales:
Dentro de la carpeta source 
•	NokiaLCD.c
Contiene las configuraciones necesarias para la pantalla LCD. Con ella se pueden construir patrones específicos a mostrar, siguiendo los caracteres declarados por medio de valores hexadecimales. El método de comunicación serial está declarada.

•	freertos_spi.c
Contiene configuraciones específicas para la comunicación SPI de FreeRTOS, así como sus funciones principales para la comunicación.

•	FreeRTOSConfig.h
Para el correcto funcionamiento del programa, se debe verificar que las siguientes macrofunciones tengan los valores indicados a continuación:
  - configUSE_PREEMPTION  ---> 1;
  - configUSE_TIME_SLICING ---> 0;
  
•	FinalProject.c
Se debe asegurar que los siguientes archivos header estén incluidos:
nokiaLCD.h
Freertos.h
task.h
semphr.h
freertos_spi.h
event_groups.h
board.h
peripherals.h
pin_mux.h
clock_config.h
MK64F12.h
fsl_debug_console.h
fsl_common.h
fsl_port.h

Las macros  ALARM_SECOND , ALARM_MINUTE  y ALARM_HOUR, tienen como objetivo definir los bits para el eventGruop, en la tarea taskAlarm, debe recibir de las correspondientes tareas taskSeconds, taskMinutes y taskHours.

El método de ajuste para la activación de la alarma es a través de las variables globales alarmSecond, alarmSeconds, alarmMinute, alarmMinutes, alarmHour y alarmHours.

En la estructura freertos_spi_handle_t se crean los dos semáforos, el primer semáforo, minutes_semaphore, tiene el propósito de desbloquear la tarea  taskMinutes, una vez que su predecesor taskSeconds, haya alcanzado un valor numérico de 60 segundos en dos variables locales a la misma. El segundo semáforo, hours_semaphore, libera la tarea taskHours, después de que taskMinutes haya completado 60 minutos. 

La estructura bi_val_t, contiene dos variables, las cuales se interpretan como las decenas y centenas correspondientes a los marcadores de segundos, minutos y horas.

Se crea la definición enumerada time_types_t, con los valores que sirven como identificador para cada tipo de estructura que es enviada desde cada tarea hacia la cola.

La estructura time_msg_t, será el tipo de dato enviado a cada elemento de la cola. Esta tiene como elementos a bi_val_t y time_types_t. 

Antes de entrar al main, se crea la variable de configuración para la comunicación SPI, el manejador de los EventGroup y se definen los prototipos de función para cada tarea.

main
En la función principal se definen las configuraciones necesarias para utilizar los elementos periféricos de la tarjeta en cuestión. La creación de la cola (queue), la asignación de elementos y tamaño por elemento, así como la creación de EventGroup, son consecutivos a las sentencias que involucran los periféricos. Por último, se realiza la creación de tareas, teniendo un puntero a void de la cola como parámetro común y una asignación de tareas que prioriza la tarea inicializadora de la pantalla LCD (taskLCDInit), posteriormente la tarea de impresión (print), luego las tareas de segundos (taskSeconds), minutos (taskSeconds) y horas (taskHours) y finalmente la tarea de alarma (taskAlarm). Por último, se declara el calendarizador y una función dummy.


taskLCDInit
Inicializa la pantalla LCD. Para controlar la posición de cualquier carácter escrito en la pantalla, se debe colocar de manera manual por medio de la función nokiaLCD_setChar(char, X, Y, Z).

taskSeconds
Se declaran dos variables locales las cuales tienen como objetivo el conteo sucesivo de los segundos, se declara una variable de tipo estructura time_msg_t asi como un puntero a este tipo de estructura, y se castea el parámetro tipo puntero a void, a un manejador de colas. Consecutivamente se pasa al ciclo sin termino, en donde se condiciona el valor de las variables locales de conteo, para acotar sus valores a menor de 60 segundos, teniendo además el objetivo de la liberación del semáforo de la siguiente tarea. Antes de finalizar la tarea se hace un llenado de la variable de tipo estructura, se almacena un espacio de memoria del tamaño de dicha estructura y se hace una copia de los valores llenados inicialmente, para finalmente pasar la dirección de memoria de la memoria dinámica reservada. Es importante aclarar que el incremento de la variable de unidades (seconds) se hace cada iteración del ciclo for, y la de decenas cada ocasión que se presenta el caso en que seconds es mayor a 9. Cabe mencionar que el único Delay en el programa se encuentra como última sentencia en la tarea taskSeconds.

taskMinutes y taskHours
El principio de comportamiento de estas tareas es similar a la antes mencionada, con la diferencia de que en estas existe un semáforo que espera a que el orden de la ejecución de tareas sea cumplido.  

taskAlarm
Se crea el evento y la máscara de bits que el EventGroup debe esperar. En el ciclo for se espera a que el grupo de eventos sea cumplido, para finalmente mostrar en pantalla la palabra ALARMA. La palabra escrita a la hora de mostrar la alarma queda definida con variables locales, alarmX, de tipo char en la tarea taskAlarm.

taskPrint
Realiza un despliegue de valores iniciales en la pantalla, posteriormente se crea un receptor de tipo puntero a estructura que servirá para almacenar el dato extraído de la cola, y finalmente se realiza una identificación del tipo unidades que se deben imprimir, por medio de una sentencia switch que compara los identificadores numéricos de cada elemento de la estructura almacenado en la cola.
