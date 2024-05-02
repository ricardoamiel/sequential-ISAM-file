# Acceso a Archivo Secuencial (Sequential File) 

El acceso a archivos secuenciales es uno de los métodos más simples para almacenar y recuperar datos. En este método, los registros se almacenan uno tras otro en orden secuencial, generalmente basado en una clave de búsqueda (search key). 

## Ventajas: 

**1. Eficiencia en la inserción en bloque:** Si los datos ya están ordenados, insertarlos en bloque es muy eficiente. 
**2. Acceso secuencial rápido:** Leer registros secuencialmente es muy eficiente, lo cual es ideal para operaciones que requieren procesar grandes volúmenes de datos de manera secuencial.

## Desventajas: 

**1. Inserciones, eliminaciones y actualizaciones individuales lentas:** Cada vez que se inserta o elimina un registro, puede requerirse un resize del archivo, especialmente si está ordenado. Por lo que una ‘solución’ es usar un archivo auxiliar, pero eso lo veremos más adelante.
**2. Búsqueda puntual y por rango no optimizadas:** Aunque el binary search es efectivo, puede ser muy lenta para archivos grandes ya que, en el peor caso, puede requerir recorrer todo el archivo.

# Acceso a Archivo con Índice Disperso (Sparse Index File)

El índice disperso (o índice ISAM) es una técnica más compleja que implica mantener un índice separado que apunta a los registros en el archivo de datos. Este índice está típicamente organizado en múltiples niveles, donde cada nivel reduce el conjunto de datos a examinar. 

## Ventajas: 

**1. Búsquedas rápidas:** Permite accesos rápidos a los datos mediante el índice, especialmente útil para archivos grandes. 
**2. Eficiencia en búsquedas puntuales y por rango:** Al reducir el espacio de búsqueda a través del índice, estas operaciones son generalmente más rápidas que en los archivos secuenciales. 
**3. Escalabilidad:** Maneja mejor grandes volúmenes de datos y es más eficiente en entornos donde las consultas son comunes y cruciales. 

## Desventajas:

**1. Complejidad:** Más difícil de implementar y mantener. 
**2. Overhead de mantenimiento del índice:** El índice necesita actualizarse con cada inserción o eliminación, lo cual puede ser costoso.
**3. Espacio adicional requerido para el índice:** Se requiere más memoria para almacenar el índice adicional. 

# SEQUENTIAL FILE (IMPLEMENTADO)

## Estructura y Funcionalidad 

1. Struct Registro y Sobrecarga de Operadores: 

Utilizas una estructura para definir los campos del registro y sobrecargas los operadores de comparación, lo cual es útil para realizar ordenamientos y comparaciones directamente usando estos operadores. La función get_key permite acceder de manera simplificada a la clave del registro, que es esencial para las operaciones de búsqueda. 

2. Clase SequentialFile: 

**Inserción (insertAll y add):** La función insertAll toma un vector de registros, los ordena y los escribe en el archivo principal. La función add agrega registros en un archivo auxiliar y, cuando alcanza un umbral basado en el logaritmo(base 2) del número de registros principales, fusiona los archivos auxiliares y principales reordenándolos. Esta estrategia ayuda a manejar inserciones frecuentes minimizando la reescritura del archivo principal. 

**Búsqueda (search):** Implementas una búsqueda binaria, que es eficiente sobre datos ordenados, para encontrar un registro específico. Si no lo encuentra en el archivo principal, busca en el auxiliar. 

**Búsqueda por Rango (rangeSearch):** La función busca el rango especificado de claves de registro y devuelve todos los registros dentro de ese rango, considerando ambos archivos (ocupas búsqueda binaria para encontrar el begin y a partir de ello buscan en el rango). 

## Rendimiento y Escalabilidad 

### Ventajas: 

**Eficiencia de Lectura:** Las operaciones de búsqueda son eficientes gracias al uso de la búsqueda binaria en un archivo ordenado. 

**Gestión de Inserciones:** La inserción en el archivo auxiliar y la fusión posterior al alcanzar un umbral es una buena estrategia para balancear entre rendimiento de inserción y la sobrecarga de mantener el archivo ordenado. 

### Desventajas: 

**Manejo de Archivos Auxiliares:** Mantener un archivo auxiliar requiere un manejo cuidadoso para evitar inconsistencias y pérdida de datos, especialmente en situaciones de fallo. 

**Rendimiento de Escritura:** Aunque las inserciones individuales son rápidas gracias al archivo auxiliar, la operación de fusión y reordenamiento puede ser costosa (especialmente si nuestro sortR tiene una complejidad alta para archivos con muchos datos), especialmente a medida que crece el tamaño del archivo. 
