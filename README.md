# IconWear KDE

Una aplicación para KDE Plasma que muestra el desgaste visual en los iconos según su uso. Cuanto más uses una aplicación (clics/lanzamientos), más "gastado" se verá su icono.

<div align="center">

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![KDE](https://img.shields.io/badge/KDE-Plasma-0051BA.svg)](https://www.kde.org/)
[![Qt](https://img.shields.io/badge/Qt-5.15+-green.svg)](https://www.qt.io/)

### Logo del Proyecto

![IconWear Logo](./assets/logo.png)

</div>

---

## Descripción

Los íconos envejecen según cuánto los usás. Tu escritorio cuenta tu historia.

## 📸 Capturas de Pantalla

### Interfaz Principal
![Screenshot 1](./assets/screenshot_1.png)

### Desgaste Visual en Iconos
![Screenshot 2](./assets/screenshot_2.png)

### Widget de Plasma
![Screenshot 3](./assets/screenshot_3.png)

---

## Estructura del Proyecto

- `src/daemon/`: Servicio en segundo plano que rastrea el uso de aplicaciones mediante `KActivitiesStats` y señales de DBus.
- `src/plasmoid/`: Widget de Plasma que demuestra el efecto de desgaste utilizando Shaders de QML.
- `assets/`: Texturas y recursos visuales para el efecto de desgaste.

## Requisitos

Consulta el archivo [DEPENDENCIES.md](DEPENDENCIES.md) para obtener una lista detallada de los paquetes necesarios según tu distribución de Linux.

## Compilación e Instalación

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

## Cómo funciona

1. El **Daemon** escucha la señal `ResourceOpened` de `org.kde.ActivityManager`.
2. Cada vez que se abre una aplicación, se incrementa su nivel de "desgaste" en una base de datos interna (o memoria por ahora).
3. El **Plasmoid** consulta el nivel de desgaste vía DBus y aplica un `ShaderEffect` sobre el icono de la aplicación.
4. El shader añade ruido (arañazos) y desaturación proporcional al nivel de uso.

## Futuras Mejoras

- [ ] Persistencia de datos (guardar niveles de desgaste en un archivo de config).
- [ ] Integración global con el Task Manager oficial de Plasma.
- [ ] Diferentes tipos de desgaste (suciedad, grietas, decoloración).
- [ ] Configuración de la velocidad de desgaste.

---

## 👨‍💻 Autor

**Nicolas Butterfield**  
📧 [nicobutter@gmail.com](mailto:nicobutter@gmail.com)

---

## 📄 Licencia

Este proyecto está bajo la licencia MIT. Consulta el archivo [LICENSE](LICENSE) para más detalles.

---

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Para cambios importantes, abre un issue primero para discutir qué te gustaría cambiar.
