# 📋 Especificación de Requerimientos de Software (SRS)

## IconWear KDE

**Versión:** 1.0  
**Fecha:** 18 de Diciembre de 2025  
**Autor:** Nicolas Butterfield  
**Email:** nicobutter@gmail.com  
**Estado:** Draft

---

## 📑 Tabla de Contenidos

1. [Introducción](#1-introducción)
2. [Descripción General](#2-descripción-general)
3. [Requerimientos Específicos](#3-requerimientos-específicos)
4. [Requerimientos de Interfaz](#4-requerimientos-de-interfaz)
5. [Requerimientos No Funcionales](#5-requerimientos-no-funcionales)
6. [Restricciones de Diseño](#6-restricciones-de-diseño)
7. [Atributos del Sistema](#7-atributos-del-sistema)
8. [Apéndices](#8-apéndices)

---

## 1. Introducción

### 1.1 Propósito

Este documento describe los requerimientos funcionales y no funcionales del sistema **IconWear KDE**, una aplicación para el entorno de escritorio KDE Plasma que implementa un sistema de desgaste visual progresivo para iconos de aplicaciones basado en patrones de uso.

El documento está destinado a:
- Desarrolladores del proyecto
- Testers y QA
- Stakeholders técnicos
- Contribuidores de la comunidad open-source

### 1.2 Alcance del Producto

**IconWear KDE** es un sistema compuesto por:

1. **Daemon de Backend** (`iconwear-daemon`): Servicio en segundo plano que rastrea el uso de aplicaciones y calcula niveles de desgaste.

2. **Widget de Plasma** (`iconwear-plasmoid`): Componente visual que renderiza iconos con efectos de desgaste usando shaders GLSL.

**Objetivos del Sistema:**
- Añadir una dimensión temporal y narrativa al escritorio del usuario
- Visualizar patrones de uso de aplicaciones de forma elegante
- Proporcionar una experiencia visual única y satisfactoria
- Respetar la privacidad del usuario (datos 100% locales)

**Beneficios Esperados:**
- Experiencia de usuario diferenciada
- Consciencia visual del uso de aplicaciones
- Estética retro/nostálgica del "desgaste digital"

### 1.3 Definiciones, Acrónimos y Abreviaturas

| Término | Definición |
|---------|------------|
| **DBus** | Sistema de comunicación inter-procesos en Linux |
| **KDE** | K Desktop Environment - Entorno de escritorio Linux |
| **Plasma** | Framework de widgets de KDE |
| **Plasmoid** | Widget nativo de KDE Plasma |
| **QML** | Qt Modeling Language - Lenguaje declarativo para UIs |
| **GLSL** | OpenGL Shading Language - Lenguaje para shaders |
| **Shader** | Programa que se ejecuta en GPU para efectos visuales |
| **Desgaste** | Nivel de "envejecimiento" visual de un icono (0-100) |
| **Reconstrucción** | Acción de resetear el desgaste de un icono |
| **KConfig** | Sistema de configuración de KDE |
| **ActivityManager** | Servicio de KDE que rastrea actividades del usuario |

### 1.4 Referencias

| Documento | Descripción |
|-----------|-------------|
| IEEE 830-1998 | Práctica recomendada para SRS |
| KDE Frameworks Documentation | https://api.kde.org/ |
| Qt 5 Documentation | https://doc.qt.io/qt-5/ |
| DBus Specification | https://dbus.freedesktop.org/doc/dbus-specification.html |
| ARCHITECTURE.md | Documento de arquitectura técnica del proyecto |

### 1.5 Visión General del Documento

- **Sección 2:** Descripción general del producto y contexto
- **Sección 3:** Requerimientos funcionales detallados
- **Sección 4:** Interfaces externas e internas
- **Sección 5:** Requerimientos no funcionales (rendimiento, seguridad)
- **Sección 6:** Restricciones técnicas y de diseño
- **Sección 7:** Atributos de calidad del sistema
- **Sección 8:** Apéndices y material complementario

---

## 2. Descripción General

### 2.1 Perspectiva del Producto

IconWear KDE es un **producto standalone** que se integra con el ecosistema KDE Plasma existente. No reemplaza ningún componente del sistema sino que añade funcionalidad complementaria.

```
┌─────────────────────────────────────────────────────────┐
│                    KDE Plasma Desktop                    │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────┐  │
│  │ Task Manager │    │ IconWear     │    │ System   │  │
│  │ (Existente)  │    │ Plasmoid     │    │ Tray     │  │
│  │              │    │ (NUEVO)      │    │          │  │
│  └──────────────┘    └──────┬───────┘    └──────────┘  │
│                             │                           │
│                        DBus │                           │
│                             │                           │
│  ┌──────────────────────────┴───────────────────────┐  │
│  │           iconwear-daemon (NUEVO)                │  │
│  │                                                   │  │
│  │  Escucha: org.kde.ActivityManager               │  │
│  │  Expone:  org.kde.iconwear                      │  │
│  └───────────────────────────────────────────────────┘  │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

**Relaciones con otros sistemas:**
- **KDE ActivityManager:** Fuente de eventos de uso de aplicaciones
- **KConfig:** Sistema de persistencia de datos
- **Qt/QML:** Framework de UI
- **DBus Session Bus:** Canal de comunicación

### 2.2 Funciones del Producto

| ID | Función | Descripción |
|----|---------|-------------|
| F01 | Rastreo de lanzamientos | Detectar cuando el usuario abre aplicaciones |
| F02 | Rastreo de tiempo activo | Acumular tiempo de uso por aplicación |
| F03 | Cálculo de desgaste | Aplicar fórmula ponderada de desgaste |
| F04 | Visualización de desgaste | Renderizar iconos con efectos visuales |
| F05 | Reseteo de desgaste | Permitir "reconstruir" iconos a estado original |
| F06 | Persistencia de datos | Guardar y cargar estado entre sesiones |
| F07 | API DBus | Exponer interfaz para consultas remotas |
| F08 | Tooltip informativo | Mostrar estadísticas al pasar mouse |
| F09 | Menú contextual | Opciones de reset y visualización |

### 2.3 Características del Usuario

| Tipo de Usuario | Descripción | Nivel Técnico |
|-----------------|-------------|---------------|
| **Usuario Final** | Usuario de KDE Plasma que desea personalización visual | Bajo-Medio |
| **Power User** | Usuario avanzado que quiere consultar métricas via DBus | Medio-Alto |
| **Desarrollador** | Integrador que quiere extender el sistema | Alto |

**Perfil del Usuario Típico:**
- Usa KDE Plasma como entorno de escritorio principal
- Aprecia personalización y estética del escritorio
- Valora privacidad (datos locales)
- Disfruta de feedback visual del sistema

### 2.4 Restricciones Generales

| Restricción | Descripción |
|-------------|-------------|
| **Plataforma** | Solo Linux con KDE Plasma 5.x |
| **Dependencias** | Requiere Qt 5.15+, KF5 5.90+ |
| **Lenguaje** | C++ (daemon), QML (plasmoid) |
| **Licencia** | MIT License |
| **Privacidad** | Datos 100% locales, sin telemetría |

### 2.5 Suposiciones y Dependencias

**Suposiciones:**
- El usuario tiene KDE Plasma instalado y funcional
- ActivityManager está corriendo y emitiendo señales
- El usuario tiene permisos para instalar servicios de usuario
- GPU soporta shaders GLSL básicos (OpenGL 2.0+)

**Dependencias Externas:**
- `org.kde.ActivityManager` debe estar disponible en DBus
- KConfig framework debe estar funcional
- Qt Quick Scene Graph debe estar operativo

---

## 3. Requerimientos Específicos

### 3.1 Requerimientos Funcionales

#### 3.1.1 Módulo: Rastreo de Uso (UsageTracker)

| ID | Requerimiento | Prioridad | Estado |
|----|---------------|-----------|--------|
| **RF-001** | El sistema DEBE escuchar la señal `ResourceOpened` de `org.kde.ActivityManager` | Alta | ✅ Implementado |
| **RF-002** | El sistema DEBE extraer el identificador de aplicación (appId) del recurso abierto | Alta | ✅ Implementado |
| **RF-003** | El sistema DEBE incrementar el contador de lanzamientos por cada evento `ResourceOpened` | Alta | ✅ Implementado |
| **RF-004** | El sistema DEBE registrar la aplicación en la lista de "activas" con timestamp | Media | ✅ Implementado |
| **RF-005** | El sistema DEBE verificar aplicaciones activas cada 30 segundos | Media | ✅ Implementado |
| **RF-006** | El sistema DEBE acumular tiempo activo (+30 seg) para apps en ejecución | Alta | ✅ Implementado |
| **RF-007** | El sistema DEBE detectar inactividad (>5 min) y marcar app como cerrada | Media | ✅ Implementado |

#### 3.1.2 Módulo: Cálculo de Desgaste

| ID | Requerimiento | Prioridad | Estado |
|----|---------------|-----------|--------|
| **RF-010** | El sistema DEBE calcular desgaste usando fórmula: `(launches × 1.0) + (activeMinutes × 0.01)` | Alta | ✅ Implementado |
| **RF-011** | El sistema DEBE normalizar el desgaste al rango 0-100 | Alta | ✅ Implementado |
| **RF-012** | El sistema DEBE emitir señal `wearLevelChanged(appId, level)` al recalcular | Alta | ✅ Implementado |
| **RF-013** | El sistema DEBE permitir configuración de factores de desgaste | Baja | ⏳ Pendiente v0.2 |

#### 3.1.3 Módulo: Reseteo (Reconstrucción)

| ID | Requerimiento | Prioridad | Estado |
|----|---------------|-----------|--------|
| **RF-020** | El sistema DEBE permitir resetear el desgaste de una aplicación a 0 | Alta | ✅ Implementado |
| **RF-021** | El sistema DEBE incrementar contador de reconstrucciones al resetear | Media | ✅ Implementado |
| **RF-022** | El sistema DEBE guardar timestamp del último reset | Baja | ✅ Implementado |
| **RF-023** | El sistema DEBE emitir señal `wearLevelReset(appId)` al resetear | Media | ✅ Implementado |

#### 3.1.4 Módulo: Persistencia

| ID | Requerimiento | Prioridad | Estado |
|----|---------------|-----------|--------|
| **RF-030** | El sistema DEBE guardar datos en `~/.config/iconwearrc` | Alta | ✅ Implementado |
| **RF-031** | El sistema DEBE cargar datos guardados al iniciar el daemon | Alta | ✅ Implementado |
| **RF-032** | El sistema DEBE guardar: wearLevel, launches, activeTimeSeconds, reconstructions | Alta | ✅ Implementado |
| **RF-033** | El sistema DEBE usar formato KConfig nativo | Media | ✅ Implementado |

#### 3.1.5 Módulo: Interfaz DBus

| ID | Requerimiento | Prioridad | Estado |
|----|---------------|-----------|--------|
| **RF-040** | El sistema DEBE registrar servicio `org.kde.iconwear` en session bus | Alta | ✅ Implementado |
| **RF-041** | El sistema DEBE exponer objeto `/Tracker` con interfaz pública | Alta | ✅ Implementado |
| **RF-042** | El sistema DEBE implementar método `getWearLevel(appId) → int` | Alta | ✅ Implementado |
| **RF-043** | El sistema DEBE implementar método `getMetrics(appId) → JSON string` | Media | ✅ Implementado |
| **RF-044** | El sistema DEBE implementar método `resetWearLevel(appId)` | Media | ✅ Implementado |
| **RF-045** | El sistema DEBE implementar método `getReconstructions(appId) → int` | Baja | ✅ Implementado |

#### 3.1.6 Módulo: Visualización (Plasmoid)

| ID | Requerimiento | Prioridad | Estado |
|----|---------------|-----------|--------|
| **RF-050** | El widget DEBE mostrar icono con efecto de desgaste visual | Alta | ✅ Implementado |
| **RF-051** | El widget DEBE aplicar shader GLSL para efectos de scratches | Alta | ✅ Implementado |
| **RF-052** | El widget DEBE aplicar desaturación proporcional al desgaste | Media | ✅ Implementado |
| **RF-053** | El widget DEBE mostrar tooltip con estadísticas al hacer hover | Media | ✅ Implementado |
| **RF-054** | El widget DEBE mostrar menú contextual con clic derecho | Media | ✅ Implementado |
| **RF-055** | El widget DEBE ejecutar animación de "spark" al resetear | Baja | ✅ Implementado |

### 3.2 Requerimientos de Datos

#### 3.2.1 Estructura de Datos: AppWearInfo

```cpp
struct AppWearInfo {
    int wearLevel;           // 0-100, nivel de desgaste normalizado
    int launches;            // Contador de lanzamientos
    qint64 activeTimeSeconds;// Tiempo activo acumulado (segundos)
    int reconstructions;     // Contador de resets
    QDateTime lastOpenTime;  // Timestamp última apertura
    QDateTime lastResetTime; // Timestamp último reset
};
```

#### 3.2.2 Formato de Métricas JSON

```json
{
    "appId": "firefox",
    "wearLevel": 45,
    "launches": 12,
    "activeMinutes": 144,
    "reconstructions": 2,
    "lastOpenTime": "2025-12-18T15:32:45",
    "lastResetTime": "2025-12-17T10:00:00"
}
```

#### 3.2.3 Formato de Configuración KConfig

```ini
[Applications]

[Applications/firefox]
wearLevel=45
launches=12
activeTimeSeconds=8640
reconstructions=2
```

---

## 4. Requerimientos de Interfaz

### 4.1 Interfaces de Usuario

#### 4.1.1 Widget Principal

| Elemento | Descripción | Interacción |
|----------|-------------|-------------|
| **Icono** | Icono de aplicación con shader de desgaste | Visual |
| **Tooltip** | Panel flotante con estadísticas | Hover |
| **Menú Contextual** | Opciones de reset y métricas | Clic derecho |
| **Slider** | Control para testing manual (dev only) | Drag |

#### 4.1.2 Tooltip de Estadísticas

```
┌─────────────────────────┐
│ Uso: 347 aperturas      │
│ Desgaste: 78%           │
│ Activo: 45 min          │
│ Restauraciones: 2       │
└─────────────────────────┘
```

#### 4.1.3 Menú Contextual

```
┌─────────────────────────┐
│ 📊 Ver Métricas        │
├─────────────────────────┤
│ ↩️  Restaurar Icono     │
└─────────────────────────┘
```

### 4.2 Interfaces de Hardware

| Componente | Requerimiento |
|------------|---------------|
| **GPU** | OpenGL 2.0+ para shaders GLSL |
| **Memoria** | ~10MB RAM para daemon |
| **Disco** | ~1MB para configuración |

### 4.3 Interfaces de Software

#### 4.3.1 Interfaz con ActivityManager

| Aspecto | Valor |
|---------|-------|
| **Servicio** | org.kde.ActivityManager |
| **Ruta** | /Resources |
| **Interfaz** | org.kde.ActivityManager.Resources |
| **Señal** | ResourceOpened(activity, agent, resource) |

#### 4.3.2 Interfaz DBus Expuesta

| Aspecto | Valor |
|---------|-------|
| **Servicio** | org.kde.iconwear |
| **Ruta** | /Tracker |
| **Interfaz** | org.kde.iconwear.Tracker |

**Métodos:**
```
int getWearLevel(QString appId)
QString getMetrics(QString appId)
void resetWearLevel(QString appId)
int getReconstructions(QString appId)
```

**Señales:**
```
wearLevelChanged(QString appId, int newLevel)
wearLevelReset(QString appId)
```

### 4.4 Interfaces de Comunicación

| Protocolo | Uso |
|-----------|-----|
| **DBus Session Bus** | Comunicación daemon ↔ plasmoid |
| **Qt Signals/Slots** | Comunicación interna |
| **KConfig** | Persistencia de datos |

---

## 5. Requerimientos No Funcionales

### 5.1 Rendimiento

| ID | Requerimiento | Métrica | Estado |
|----|---------------|---------|--------|
| **RNF-001** | El daemon DEBE usar menos de 1% CPU en idle | < 1% CPU | ✅ |
| **RNF-002** | El daemon DEBE usar menos de 20MB RAM | < 20MB | ✅ |
| **RNF-003** | El shader DEBE renderizar a 60 FPS | 60 FPS | ✅ |
| **RNF-004** | La respuesta DBus DEBE ser < 10ms | < 10ms | ✅ |
| **RNF-005** | El chequeo de apps activas DEBE ejecutar en < 5ms | < 5ms | ✅ |

### 5.2 Seguridad

| ID | Requerimiento | Estado |
|----|---------------|--------|
| **RNF-010** | El sistema NO DEBE transmitir datos a servidores externos | ✅ |
| **RNF-011** | Los datos DEBEN almacenarse solo en directorio del usuario | ✅ |
| **RNF-012** | El daemon NO DEBE requerir privilegios de root | ✅ |
| **RNF-013** | La interfaz DBus DEBE usar session bus (no system) | ✅ |

### 5.3 Fiabilidad

| ID | Requerimiento | Métrica |
|----|---------------|---------|
| **RNF-020** | El daemon DEBE recuperarse de errores DBus sin crash | 99.9% uptime |
| **RNF-021** | Los datos DEBEN persistir correctamente entre reinicios | 100% integridad |
| **RNF-022** | El daemon DEBE manejar apps desconocidas gracefully | Sin exceptions |

### 5.4 Disponibilidad

| ID | Requerimiento |
|----|---------------|
| **RNF-030** | El daemon DEBE iniciar automáticamente con la sesión |
| **RNF-031** | El plasmoid DEBE funcionar sin daemon (modo degradado) |

### 5.5 Mantenibilidad

| ID | Requerimiento |
|----|---------------|
| **RNF-040** | El código DEBE estar documentado con Doxygen |
| **RNF-041** | El proyecto DEBE compilar sin warnings |
| **RNF-042** | El código DEBE seguir Qt/KDE coding guidelines |

### 5.6 Portabilidad

| ID | Requerimiento |
|----|---------------|
| **RNF-050** | El sistema DEBE compilar en Fedora, Ubuntu, Arch |
| **RNF-051** | El sistema DEBE funcionar con Qt 5.15+ |
| **RNF-052** | El sistema DEBE funcionar con KF5 5.90+ |

---

## 6. Restricciones de Diseño

### 6.1 Restricciones Técnicas

| Restricción | Justificación |
|-------------|---------------|
| **Solo C++/QML** | Integración nativa con KDE |
| **Solo DBus** | Estándar de IPC en Linux desktop |
| **Solo KConfig** | Consistencia con ecosistema KDE |
| **Sin base de datos externa** | Simplicidad y privacidad |

### 6.2 Restricciones de Implementación

| Restricción | Detalle |
|-------------|---------|
| **Fórmula de desgaste** | Debe ser ponderada (launches + tiempo) |
| **Timer interval** | Fijo en 30 segundos para balance rendimiento/precisión |
| **Timeout inactividad** | 5 minutos para detectar cierre de app |
| **Max wear level** | Capped a 100 para normalización |

### 6.3 Estándares Aplicables

| Estándar | Aplicación |
|----------|------------|
| **KDE HIG** | Human Interface Guidelines para UI |
| **Qt Coding Style** | Convenciones de código |
| **Doxygen** | Formato de documentación |
| **Semantic Versioning** | Versionado del proyecto |

---

## 7. Atributos del Sistema

### 7.1 Usabilidad

- **Instalación:** Un comando (make install)
- **Configuración:** Zero-config (funciona out-of-the-box)
- **Aprendizaje:** Intuitivo (visual, sin configuración necesaria)
- **Accesibilidad:** Tooltips con texto descriptivo

### 7.2 Escalabilidad

- **Aplicaciones:** Sin límite teórico
- **Datos:** Proporcional a número de apps usadas
- **Memoria:** Crece linealmente con apps rastreadas

### 7.3 Extensibilidad

- **API DBus:** Permite integración con herramientas externas
- **Factores configurables:** Planeado para v0.2
- **Temas de desgaste:** Planeado para v0.3

### 7.4 Testabilidad

- **Unit tests:** Pendiente implementación
- **Integration tests:** Pendiente implementación
- **Manual testing:** Slider en UI para simular desgaste

---

## 8. Apéndices

### 8.1 Diagrama de Casos de Uso

```
                    ┌─────────────────────────────────────┐
                    │           IconWear KDE              │
                    └─────────────────────────────────────┘
                                     │
         ┌───────────────────────────┼───────────────────────────┐
         │                           │                           │
         ▼                           ▼                           ▼
┌─────────────────┐       ┌─────────────────┐       ┌─────────────────┐
│   Ver Desgaste  │       │  Restaurar      │       │  Ver Métricas   │
│   de Icono      │       │  Icono          │       │  de Uso         │
└────────┬────────┘       └────────┬────────┘       └────────┬────────┘
         │                         │                         │
         │                         │                         │
         └─────────────────────────┼─────────────────────────┘
                                   │
                                   ▼
                            ┌──────────────┐
                            │   Usuario    │
                            │   Final      │
                            └──────────────┘
```

### 8.2 Diagrama de Estados del Desgaste

```
┌───────────┐     launch      ┌───────────┐
│  Nuevo    │ ──────────────► │  En Uso   │
│  (0%)     │                 │  (1-99%)  │
└───────────┘                 └─────┬─────┘
      ▲                             │
      │         reset               │ uso continuo
      │     ┌───────────────────────┘
      │     │
      │     ▼
      │   ┌───────────┐
      └───│ Desgastado│
          │  (100%)   │
          └───────────┘
```

### 8.3 Matriz de Trazabilidad

| Caso de Uso | Requerimientos Funcionales |
|-------------|---------------------------|
| Ver Desgaste | RF-050, RF-051, RF-052 |
| Restaurar Icono | RF-020, RF-021, RF-023, RF-055 |
| Ver Métricas | RF-043, RF-053, RF-054 |
| Rastreo Automático | RF-001 a RF-007 |
| Persistencia | RF-030 a RF-033 |

### 8.4 Glosario Extendido

| Término | Definición Técnica |
|---------|-------------------|
| **appId** | Identificador único de aplicación (ej: "firefox", "org.kde.dolphin") |
| **Wear Factor** | Constante multiplicadora para cálculo de desgaste |
| **Reconstruction** | Evento de reset que pone desgaste en 0 pero mantiene historial |
| **Spark Animation** | Efecto visual de destello blanco durante reset |
| **Session Bus** | Canal DBus específico de la sesión de usuario |

### 8.5 Historial de Revisiones

| Versión | Fecha | Autor | Cambios |
|---------|-------|-------|---------|
| 1.0 | 2025-12-18 | Nicolas Butterfield | Documento inicial completo |

---

## 📝 Notas Finales

Este documento de especificación de requerimientos está sujeto a cambios conforme evolucione el proyecto. Todas las modificaciones significativas serán versionadas y documentadas en la sección de historial de revisiones.

Para preguntas o sugerencias sobre este documento, contactar a:
- **Email:** nicobutter@gmail.com
- **Proyecto:** https://github.com/lordcommander/iconwear-kde

---

**Documento generado según estándar IEEE 830-1998**  
**© 2025 Nicolas Butterfield - MIT License**
