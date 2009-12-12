#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QImage>
#include <QPixmap>
#include <QHash>
#include <QSettings>
#include <QTableWidget>
#include <QLabel>

#include "types.h"
#include "inputimage.h"
#include "lvkframe.h"
#include "lvkanimation.h"
#include "lvkaframe.h"
#include "spritestate.h"
#include "lvkframegraphicsgroup.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    /// Opens an sprite file
    /// @returns true if success, false otherwise
    bool openFile(const QString& filename, SpriteState::SpriteStateError* err = 0);

    /// Exports an sprite file to its binary format
    /// @returns true if success, false otherwise
    bool exportFile(const QString& filename);
    
private:

    Ui::MainWindow *ui;

    /// current file open
    QString  _filename;

    /// current export file open
    QString _exportFileName;

    /// app settings
    QSettings settings;

    /// sets current file and updates the main window title.
	/// The filename will be stored in the recent files section in the config file
    void setCurrentFile(const QString& filename);

    /// sets current export file
    void setCurrentExportFile(const QString& exportFileName);

    /// current sprite state
    SpriteState _sprState;

    /// Counter. Next image id
    Id _imgId;

    /// Counter. Next frame id
    Id _frameId;

    /// Counter. Next animation id
    Id _aniId;

    /// Counter. Next animation frame id
    Id _aframeId;

    /// Current animation
    LvkFrameGraphicsGroup* currentAnimation;

    /// Labels to show information in the status bar
    QLabel* statusBarMousePos;
    QLabel* statusBarRectSize;

    /// initialize recent files menu
    void initRecentFilesMenu();

    /// initialize signals
    void initSignals();

    /// initialize tables
    void initTables();

    /// connect or disconnect cellChanged() signals
    void cellChangedSignals(bool connected);

    /// Add new input image
    void addImage(const InputImage& image);

    /// Add new frame
    void addFrame(const LvkFrame& frame);

    /// Add new animation
    void addAnimation(const LvkAnimation& ani);

    /// Add new animation frame to the animation @param aniId
    void addAframe(const LvkAframe& aframe, Id aniId);
    void addAframe_(const LvkAframe& aframe, Id aniId);

    /// shorthand to handle tables
    inline QString getItem(const QTableWidget* table, int row, int col)
    { return table->item(row, col)->text(); }

    /// shorthand to handle tables
    inline Id getIdItem(const QTableWidget* table, int row, int col)
    { return (Id)(getItem(table, row, col).toInt()); }

    /// shorthand to handle tables
    inline void setItem(const QTableWidget* table, int row, int col, const QString& value)
    { cellChangedSignals(false); table->item(row, col)->setText(value); cellChangedSignals(true); }

    /// shorthand to handle tables
    inline void setItem(const QTableWidget* table, int row, int col, int value)
    { setItem(table, row, col, QString::number(value)); }

private slots:
    void saveFile();
    void saveAsFile();
    void openFileDialog();
    void closeFile();
    void exportFile();
    void exportAsFile();
    void exit();

    void addRecentFileMenu(const QString& filename);
    void storeRecentFile(const QString& filename);

    void about();

    void addImageDialog();
    void showSelImage(int row);
    void showSelImageWithFrameRect(int row, const QRect& rect);
    void removeSelImage();
    void removeImage(int row);

    bool addFrameFromImgRegion();
    void showSelFrame(int row);
    void removeSelFrame();
    void removeFrame(int row);

    void addAnimationDialog();
    void showAframes(int row);
    void removeSelAnimation();
    void removeAnimation(int row);

    void previewAnimation();
    void clearPreviewAnimation();
    void incAniSpeed(int ms = 10);
    void decAniSpeed(int ms = 10);

    void addAframeDialog();
    void showSelAframe(int row);
    void removeSelAframe();
    void removeAframe(int row);

    void updateImgTable(int row, int col);
    void updateFramesTable(int row, int col);
    void updateAframesTable(int row, int col);
    void updateAniTable(int row, int col);

    void showMousePosition(int x, int y);
    void showMouseRect(const QRect& rect);
};

#endif // MAINWINDOW_H
