#include "boot_manager/gui/main_window.h"
#include "boot_manager/core/boot_flow.h"

#include <QApplication>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtConcurrent/QtConcurrentRun>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      backend_(create_boot_backend()),
      entries_(new QListWidget),
      status_(new QLabel),
      refresh_button_(new QPushButton("重新整理")),
      set_button_(new QPushButton("設定並重新啟動"))
{
    setWindowTitle(QStringLiteral("Boot Manager %1").arg(QApplication::applicationVersion()));
    resize(520, 420);

    auto *root = new QWidget;
    auto *layout = new QVBoxLayout(root);
    layout->addWidget(new QLabel("選擇下一次啟動的 UEFI 項目"));
    layout->addWidget(new QLabel("名稱來自韌體項目或 EFI 路徑；系統版本通常無法辨識。"));
    layout->addWidget(entries_);
    layout->addWidget(refresh_button_);
    layout->addWidget(set_button_);
    layout->addWidget(status_);
    setCentralWidget(root);

    connect(refresh_button_, &QPushButton::clicked, this,
            [this]
            { refresh(); });
    connect(set_button_, &QPushButton::clicked, this,
            [this]
            { choose(); });
    connect(&scan_watcher_,
            &QFutureWatcher<QPair<QVector<BootEntry>, QString>>::finished,
            this, [this]
            {
                const auto result = scan_watcher_.result();
                entries_->clear();
                for (const auto &entry : result.first) {
                    auto *item = new QListWidgetItem(
                        QString("%1 — %2 (%3)")
                            .arg(entry.name, entry.efi_path, entry.id),
                        entries_);
                    item->setData(Qt::UserRole, entry.id);
                    item->setData(Qt::UserRole + 1, entry.name);
                    item->setData(Qt::UserRole + 2, entry.efi_path);
                }

                status_->setText(
                    result.second.isEmpty()
                        ? QString("目前系統：%1").arg(backend_->current_system())
                        : result.second);
                set_busy(false); });
    connect(&prepare_watcher_,
            &QFutureWatcher<OperationResult>::finished, this, [this]
            {
                const auto result = prepare_watcher_.result();
                set_busy(false);
                if (!result.ok) {
                    QMessageBox::critical(
                        this, "設定或驗證失敗",
                        result.message + "\n未執行重新啟動");
                    return;
                }
                confirm_reboot(); });

    refresh();
}

void MainWindow::set_busy(const bool busy)
{
    refresh_button_->setEnabled(!busy);
    set_button_->setEnabled(!busy);
    entries_->setEnabled(!busy);
}

void MainWindow::refresh()
{
    if (!backend_)
    {
        return;
    }

    set_busy(true);
    status_->setText("正在讀取韌體開機項目…");
    auto backend = backend_;
    scan_watcher_.setFuture(QtConcurrent::run([backend]
                                              {
        QString error;
        const auto entries = backend->list_entries(error);
        return qMakePair(entries, error); }));
}

void MainWindow::choose()
{
    auto *item = entries_->currentItem();
    if (!item)
    {
        QMessageBox::warning(this, "Boot Manager", "請先選擇開機項目");
        return;
    }

    const auto answer = QMessageBox::question(
        this, "確認開機目標",
        QString("即將把「%1」設為下一次開機目標。請先儲存工作，是否繼續？")
            .arg(item->data(Qt::UserRole + 1).toString()));
    if (answer != QMessageBox::Yes)
    {
        return;
    }

    BootEntry entry{item->data(Qt::UserRole).toString(),
                    item->data(Qt::UserRole + 1).toString(),
                    item->data(Qt::UserRole + 2).toString(),
                    {}};
    set_busy(true);
    status_->setText("正在設定並讀回驗證…");
    auto backend = backend_;
    prepare_watcher_.setFuture(QtConcurrent::run([backend, entry]
                                                 { return prepare_next_boot(*backend, entry); }));
}

void MainWindow::confirm_reboot()
{
    const auto answer = QMessageBox::question(
        this, "確認重新啟動", "已讀回驗證下一次開機項目。現在重新啟動？");
    if (answer != QMessageBox::Yes)
    {
        return;
    }

    set_busy(true);
    status_->setText("正在要求重新啟動…");
    auto backend = backend_;
    auto *watcher = new QFutureWatcher<OperationResult>(this);
    connect(watcher, &QFutureWatcher<OperationResult>::finished, this,
            [this, watcher]
            {
                const auto result = watcher->result();
                status_->setText(result.message);
                if (!result.ok)
                {
                    QMessageBox::critical(this, "重新啟動失敗", result.message);
                }
                set_busy(false);
                watcher->deleteLater();
            });
    watcher->setFuture(QtConcurrent::run([backend]
                                         { return backend->reboot(); }));
}
