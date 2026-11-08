# Qt Licensing and Distribution Notes

[Traditional Chinese version](READMD-CN.md)

This project uses Qt 6 for its desktop interface. `LICENSE` was copied from `C:\Qt\Licenses\LICENSE` in a Qt Online Installer installation. It contains the full GNU GPL v3 and GNU LGPL v3 terms. **This file covers Qt; it does not declare a license for this project's source code.**

## Qt components in use

- The source code directly links against Qt Core, Qt GUI, Qt Widgets, and Qt Concurrent.
- The current Windows `windeployqt` output also includes Qt Network, Qt SVG, the platform plugin, and other plugins. The actual set may change with the Qt version and packaging result.
- The Windows package uses dynamically linked DLLs. The Ubuntu `.deb` currently relies on the distribution's Qt packages and does not bundle Qt libraries.

The [Qt licensing overview](https://doc.qt.io/qt-6/licensing.html) explains that modules may have different license terms. Check the version and files actually distributed. Qt DLLs and plugins may contain code owned by other authors. See the [Qt 6.12 third-party component list](https://doc.qt.io/qt-6/licenses-used-in-qt.html) and the installed Qt `sbom/` directory. Include the notices and license texts required for the components shipped in each release.

## Before distributing binaries

The Windows packaging script copies this directory's `LICENSE` and both language versions of these notes into `licenses/qt/` in the output folder. This provides the Qt license terms, but **does not by itself complete all obligations for the release**. If distributing under Qt's open-source license, review the [official Qt obligations](https://www.qt.io/development/open-source-lgpl-obligations), including:

1. Provide the required license and copyright notices for the Qt DLLs, plugins, and third-party runtime components actually shipped.
2. Arrange access to the complete corresponding Qt source code, including any modifications, or another valid method allowed by the applicable terms. A link to Qt's download page alone may be insufficient.
3. Allow users to replace the Qt DLLs with compatible modified versions and run the application.

If using a commercial Qt license, review the applicable agreement instead. Check these items again whenever the Qt version or package contents change.
