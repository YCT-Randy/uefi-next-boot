#pragma once

#include "boot_manager/core/boot_backend.h"

#include <QFutureWatcher>
#include <QMainWindow>
#include <QPair>
#include <QVector>
#include <memory>

class QListWidget;
class QLabel;
class QPushButton;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    std::shared_ptr<BootBackend> backend_;
    QListWidget *entries_;
    QLabel *status_;
    QPushButton *refresh_button_;
    QPushButton *set_button_;
    QFutureWatcher<QPair<QVector<BootEntry>, QString>> scan_watcher_;
    QFutureWatcher<OperationResult> prepare_watcher_;

    void refresh();
    void choose();
    void set_busy(bool busy);
    void confirm_reboot();
};
