#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QPixmap>
#include <QDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsPixmapItem>
#include <QList>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lvkaction.h"
#include "inputimage.h"
#include "lvkframe.h"
#include "lvkaframe.h"
#include "lvkframegraphicsgroup.h"
#include "settings.h"

/// imgTableWidget columns
enum {
    ColImageId          = 0,
    ColImageFilename    = 1,
};

/// framesTableWidget colums
enum {
    ColFrameId          = 0,
    ColFrameOx          = 1,
    ColFrameOy          = 2,
    ColFrameW           = 3,
    ColFrameH           = 4,
    ColFrameName        = 5,
    ColFrameImgId       = 6,
};

/// aniTableWidget columns
enum {
    ColAniId            = 0,
    ColAniName          = 1,
};

/// aframesTableWidget columns
enum {
    ColAframeId         = 0,
    ColAframeFrameId    = 1,
    ColAframeOx         = 2,
    ColAframeOy         = 3,
    ColAframeDelay      = 4,
    ColAframeAniId      = 5,
};

#define getImageId(row)         ui->imgTableWidget->item(row, ColImageId)->text().toInt()
#define getFrameId(row)         ui->framesTableWidget->item(row, ColFrameId)->text().toInt()
#define getAFrameId(row)        ui->aframesTableWidget->item(row, ColAframeId)->text().toInt()
#define getAFrameFrameId(row)   ui->aframesTableWidget->item(row, ColAframeFrameId)->text().toInt()
#define getFrameImgId(row)      ui->framesTableWidget->item(row, ColFrameImgId)->text().toInt()
#define getAnimationId(row)     ui->aniTableWidget->item(row, ColAniId)->text().toInt()

#define selectedImgId()         getImageId(ui->imgTableWidget->currentRow())
#define selectedAniId()         getAnimationId(ui->aniTableWidget->currentRow())
#define selectedAframeId()      getAFrameId(ui->aframesTableWidget->currentRow())


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _imgId(0), _frameId(0), _aniId(0), _aframeId(0),
      currentAnimation(0), statusBarMousePos(new QLabel()), statusBarRectSize(new QLabel())
{
    ui->setupUi(this);
    ui->statusBar->addWidget(statusBarMousePos);
    ui->statusBar->addWidget(statusBarRectSize);

    ui->imgPreview->setPixmap(QPixmap());

    ui->framePreview->setFrameRectVisible(false);
    ui->framePreview->setPixmap(QPixmap());
    ui->framePreview->zoomIn();
    ui->framePreview->zoomIn();

    ui->aframePreview->setFrameRectVisible(false);
    ui->aframePreview->zoomIn();
    ui->aframePreview->zoomIn();

    initSignals();
    initTables();
    initRecentFilesMenu();
    showFramesTab();
    hideFramePreview();

    resize(1204, 768);
    updateGeometry();
}

void MainWindow::initSignals()
{
    connect(ui->actionSave,            SIGNAL(triggered()),          this, SLOT(saveFile()));
    connect(ui->actionSaveAs,          SIGNAL(triggered()),          this, SLOT(saveAsFile()));
    connect(ui->actionOpen,            SIGNAL(triggered()),          this, SLOT(openFileDialog()));
    connect(ui->actionClose,           SIGNAL(triggered()),          this, SLOT(closeFile_checkUnsaved()));
    connect(ui->actionExport,          SIGNAL(triggered()),          this, SLOT(exportFile()));
    connect(ui->actionExportAs,        SIGNAL(triggered()),          this, SLOT(exportAsFile()));
    connect(ui->actionUndo,            SIGNAL(triggered()),          this, SLOT(undo()));
    connect(ui->actionRedo,            SIGNAL(triggered()),          this, SLOT(redo()));
    connect(ui->actionExit,            SIGNAL(triggered()),          this, SLOT(exit()));
    connect(ui->actionAbout,           SIGNAL(triggered()),          this, SLOT(about()));
    connect(ui->actionFramesTab,       SIGNAL(triggered()),          this, SLOT(showFramesTab()));
    connect(ui->actionAnimationsTab,   SIGNAL(triggered()),          this, SLOT(showAnimationsTab()));
    connect(ui->actionAddImage,        SIGNAL(triggered()),          this, SLOT(addImageDialog()));
    connect(ui->actionAddFrame,        SIGNAL(triggered()),          this, SLOT(addFrameDialog()));
    connect(ui->actionAddAnimation,    SIGNAL(triggered()),          this, SLOT(addAnimationDialog()));
    connect(ui->actionShowHideFramesPreview, SIGNAL(triggered()),    this, SLOT(hideShowFramePreview()));
    connect(ui->actionRemoveImage,     SIGNAL(triggered()),          this, SLOT(removeSelImage()));
    connect(ui->actionRemoveFrame,     SIGNAL(triggered()),          this, SLOT(removeSelFrame()));
    connect(ui->actionRemoveAnimation, SIGNAL(triggered()),          this, SLOT(removeSelAnimation()));
    connect(ui->actionRefreshAnimation,SIGNAL(triggered()),          this, SLOT(previewAnimation()));

    connect(ui->addImageButton,        SIGNAL(clicked()),            this, SLOT(addImageDialog()));
    connect(ui->removeImageButton,     SIGNAL(clicked()),            this, SLOT(removeSelImage()));
    connect(ui->addFrameButton,        SIGNAL(clicked()),            this, SLOT(addFrameDialog()));
    connect(ui->removeFrameButton,     SIGNAL(clicked()),            this, SLOT(removeSelFrame()));
    connect(ui->addAniButton,          SIGNAL(clicked()),            this, SLOT(addAnimationDialog()));
    connect(ui->removeAniButton,       SIGNAL(clicked()),            this, SLOT(removeSelAnimation()));
    connect(ui->refreshAniButton,      SIGNAL(clicked()),            this, SLOT(previewAnimation()));
    connect(ui->addAframeButton,       SIGNAL(clicked()),            this, SLOT(addAframeDialog()));
    connect(ui->removeAframeButton,    SIGNAL(clicked()),            this, SLOT(removeSelAframe()));
    connect(ui->aniDecSpeedButton,     SIGNAL(clicked()),            this, SLOT(decAniSpeed()));
    connect(ui->aniIncSpeedButton,     SIGNAL(clicked()),            this, SLOT(incAniSpeed()));
    connect(ui->hideFramePreviewButton,SIGNAL(clicked()),            this, SLOT(hideShowFramePreview()));

    connect(ui->imgPreview,            SIGNAL(mousePositionChanged(int,int)),  this, SLOT(showMousePosition(int,int)));
    connect(ui->framePreview,          SIGNAL(mousePositionChanged(int,int)),  this, SLOT(showMousePosition(int,int)));
    connect(ui->aframePreview,         SIGNAL(mousePositionChanged(int,int)),  this, SLOT(showMousePosition(int,int)));
    connect(ui->imgPreview,            SIGNAL(mouseRectChanged(const QRect&)), this, SLOT(showMouseRect(const QRect&)));

    connect(ui->imgZoomInButton,       SIGNAL(clicked()),  ui->imgPreview,    SLOT(zoomIn()));
    connect(ui->imgZoomOutButton,      SIGNAL(clicked()),  ui->imgPreview,    SLOT(zoomOut()));
    connect(ui->frameZoomInButton,     SIGNAL(clicked()),  ui->framePreview,  SLOT(zoomIn()));
    connect(ui->frameZoomOutButton,    SIGNAL(clicked()),  ui->framePreview,  SLOT(zoomOut()));
    connect(ui->aframeZoomInButton,    SIGNAL(clicked()),  ui->aframePreview, SLOT(zoomIn()));
    connect(ui->aframeZoomOutButton,   SIGNAL(clicked()),  ui->aframePreview, SLOT(zoomOut()));

    connect(ui->imgTableWidget,        SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(showSelImage(int)));
    connect(ui->framesTableWidget,     SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(showSelFrame(int)));
    connect(ui->aframesTableWidget,    SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(showSelAframe(int)));
    connect(ui->aniTableWidget,        SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(showAframes(int)));

    cellChangedSignals(true);
}

void MainWindow::cellChangedSignals(bool connected)
{
    if (connected) {
        connect(ui->imgTableWidget,       SIGNAL(cellChanged(int,int)), this, SLOT(updateImgTable(int,int)));
        connect(ui->framesTableWidget,    SIGNAL(cellChanged(int,int)), this, SLOT(updateFramesTable(int,int)));
        connect(ui->aframesTableWidget,   SIGNAL(cellChanged(int,int)), this, SLOT(updateAframesTable(int,int)));
        connect(ui->aniTableWidget,       SIGNAL(cellChanged(int,int)), this, SLOT(updateAniTable(int,int)));
    } else {
        disconnect(ui->imgTableWidget,    SIGNAL(cellChanged(int,int)), this, SLOT(updateImgTable(int,int)));
        disconnect(ui->framesTableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(updateFramesTable(int,int)));
        disconnect(ui->aframesTableWidget,SIGNAL(cellChanged(int,int)), this, SLOT(updateAframesTable(int,int)));
        disconnect(ui->aniTableWidget,    SIGNAL(cellChanged(int,int)), this, SLOT(updateAniTable(int,int)));
    }
}

void MainWindow::initTables()
{
    QStringList headersList;

    /* input images table */

    ui->imgTableWidget->setRowCount(0);
    ui->imgTableWidget->setColumnCount(2);
    ui->imgTableWidget->setColumnWidth(ColImageId, 30);
    headersList << tr("Id") << tr("Filename");
    ui->imgTableWidget->setHorizontalHeaderLabels(headersList);
    headersList.clear();
#ifndef DEBUG_SHOW_ID_COLS
    ui->imgTableWidget->setColumnHidden(ColImageId, true);
#endif

    /* frames table */

    ui->framesTableWidget->setRowCount(0);
    ui->framesTableWidget->setColumnCount(7);
    ui->framesTableWidget->setColumnWidth(ColFrameId, 30);
    ui->framesTableWidget->setColumnWidth(ColFrameOx, 30);
    ui->framesTableWidget->setColumnWidth(ColFrameOy, 30);
    ui->framesTableWidget->setColumnWidth(ColFrameW, 30);
    ui->framesTableWidget->setColumnWidth(ColFrameH, 30);
    headersList << tr("Id") << tr("ox") << tr("oy") << tr("w") << tr("h") << tr("Name") << tr("Image Id");
    ui->framesTableWidget->setHorizontalHeaderLabels(headersList);
    headersList.clear();
#ifndef DEBUG_SHOW_ID_COLS
    ui->framesTableWidget->setColumnHidden(ColFrameId, true);
    ui->framesTableWidget->setColumnHidden(ColFrameImgId, true);
#endif

    /* animations table */

    ui->aniTableWidget->setRowCount(0);
    ui->aniTableWidget->setColumnCount(2);
    ui->aniTableWidget->setColumnWidth(ColAniId, 30);
    headersList << "Id" << "Name";
    ui->aniTableWidget->setHorizontalHeaderLabels(headersList);
    headersList.clear();
#ifndef DEBUG_SHOW_ID_COLS
    ui->aniTableWidget->setColumnHidden(ColAniId, true);
#endif

    /* animation frames table */

    ui->aframesTableWidget->setRowCount(0);
    ui->aframesTableWidget->setColumnCount(6);
    ui->aframesTableWidget->setColumnWidth(ColAframeId, 30);
    ui->aframesTableWidget->setColumnWidth(ColAframeFrameId, 60);
    ui->aframesTableWidget->setColumnWidth(ColAframeOx, 30);
    ui->aframesTableWidget->setColumnWidth(ColAframeOy, 30);
    ui->aframesTableWidget->setColumnWidth(ColAframeDelay, 50);
    ui->aframesTableWidget->setColumnWidth(ColAframeAniId, 30);
    headersList << tr("Id") << tr("Frame Id") << tr("ox") << tr("oy") << tr("Delay") << tr("Animation Id");
    ui->aframesTableWidget->setHorizontalHeaderLabels(headersList);
    headersList.clear();
#ifndef DEBUG_SHOW_ID_COLS
    ui->aframesTableWidget->setColumnHidden(ColAframeId, true);
    ui->aframesTableWidget->setColumnHidden(ColAframeAniId, true);
#endif
}

bool MainWindow::saveFile()
{
    bool success = true;

    if (_filename.isEmpty()) {
        success = saveAsFile();
    } else {
        SpriteStateError err;
        if (!_sprState.save(_filename, &err)) {
           infoDialog(tr("Cannot save") + _filename + ". " + SpriteState::errorMessage(err));
           success = false;
        }
    }

    return success;
}

bool MainWindow::saveAsFile()
{
    static QString lastDir = "";
    bool           success = true;
    QString        filename;

    filename = QFileDialog::getOpenFileName(
            this, tr("Save file"), lastDir, "*.lvks;; *.*");

    if (filename.isNull()) {
        success = false;
    } else {
        lastDir = QFileInfo(filename).absolutePath();

        SpriteStateError err;
        if (!_sprState.save(filename, &err)) {
           infoDialog(tr("Cannot save ") + filename + ". " + SpriteState::errorMessage(err));
           success = false;
        }
        setCurrentFile(filename);
    }
    return success;
}

DialogButton MainWindow::saveChangesDialog()
{
    QString msg;

    if (_filename.isEmpty()) {
        msg = tr("Save changes to file before closing?");
    } else {
        msg = tr("Save changes to file '") + _filename + tr("' before closing?");
    }

    DialogButton button =  yesNoCancelDialog(msg);

    if (button == YesButton) {
        if (!saveFile()) {
             button = CancelButton;
        }
    }
    return button;
}

void MainWindow::openFileDialog()
{
    if (_sprState.hasUnsavedChanges()) {
        if (saveChangesDialog() == CancelButton) {
            return;
        }
    }

    static QString lastDir = "";

    QString filename;

    filename = QFileDialog::getOpenFileName(
            this, tr("Open file"), lastDir, "*.lvks;; *.*");

    if (!filename.isNull()) {
        lastDir = QFileInfo(filename).absolutePath();
        openFile(filename);
    }
}

bool MainWindow::openFile_checkUnsaved(const QString& filename)
{
    if (_sprState.hasUnsavedChanges()) {
        if (saveChangesDialog() == CancelButton) {
            return false;
        }
    }
    return openFile(filename);
}

bool MainWindow::openFile(const QString& filename)
{
    SpriteStateError err;

    if (!openFile_(filename, &err)) {
        infoDialog(tr("Cannot open ") + filename + ". " + SpriteState::errorMessage(err));
        return false;
    }
    return true;
}

bool MainWindow::openFile_(const QString& filename_, SpriteStateError* err)
{
    if (!QFile::exists(filename_)) {
        if (err) {
            *err = SpriteState::ErrFileDoesNotExist;
        }
        return false;
    }

    QString filename = QFileInfo(filename_).absoluteFilePath();

    closeFile();
    setCurrentFile(filename);

    SpriteState tmp;

    if (!tmp.load(filename, err)) {
        closeFile();
        return false;
    }

    cellChangedSignals(false);

    /* load input images */

    for (QHashIterator<Id, InputImage> it(tmp.images()); it.hasNext();) {
        it.next();
        const InputImage& image =  it.value();
        addImage(image);
        _imgId = std::max(_imgId, image.id + 1);
    }

    /* load frames */

    for (QHashIterator<Id, LvkFrame> it(tmp.frames()); it.hasNext();) {
        it.next();
        const LvkFrame& frame =  it.value();
        addFrame(frame);
        _frameId = std::max(_frameId, frame.id + 1);
    }

    /* load animations */

    for (QHashIterator<Id, LvkAnimation> it(tmp.animations()); it.hasNext();) {
        it.next();
        const LvkAnimation& ani =  it.value();
        addAnimation(ani);
        _aniId = std::max(_aniId, ani.id + 1);

        /* load aframes */

        for (QHashIterator<Id, LvkAframe> it2(it.value().aframes); it2.hasNext();) {
            it2.next();
            const LvkAframe& aframe =  it2.value();
            addAframe(aframe, ani.id);
            _aframeId = std::max(_aframeId, aframe.id + 1);
        }
    }

    _sprState.markAsSaved();

    /* UI - tables and previews */

    if (ui->imgTableWidget->rowCount() > 0) {
        ui->imgTableWidget->selectRow(0);
        showSelImage(0);
    } else {
        ui->imgPreview->setPixmap(QPixmap());
    }
    if (ui->framesTableWidget->rowCount() > 0) {
        ui->framesTableWidget->selectRow(0);
        showSelFrame(0);
    } else {
        ui->framePreview->setPixmap(QPixmap());
    }
    if (ui->aniTableWidget->rowCount() > 0) {
        ui->aniTableWidget->selectRow(0);
        showAframes(0);
    } else {
        clearPreviewAnimation();
    }
    if (ui->aframesTableWidget->rowCount() > 0) {
        ui->aframesTableWidget->selectRow(0);
        showSelAframe(0);
    } else {
        ui->aframePreview->setPixmap(QPixmap());
    }

    cellChangedSignals(true);

    return true;
}

void MainWindow::storeRecentFile(const QString& filename)
{
    #define makeKey(str, i)         { str = KEY_RECENT_FILE; str.append(QString::number(i)); }

    QString key;

    /* search if filename is already stored */
    int found = -1;
    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
        makeKey(key, i);

        QString fileAlreadyStored = settings.value(key).toString();

        if (filename == fileAlreadyStored) {
            found = i;
            break;
        }
    }

    /* if found in the first position, nothing to do */
    if (found == 0) {
        return;
    }

    /* if not found, stored it in the last position */
    if (found == -1) {
        found = MAX_RECENT_FILES - 1;
        makeKey(key, found);
        settings.setValue(key, filename);
    }

    /* swap entries */
    QString key_;
    for (int i = found; i > 0; --i) {
        /* swap(i, i-1) */
        makeKey(key, i);
        makeKey(key_, i - 1);
        QString recentFile = settings.value(key).toString();
        QString recentFile_ = settings.value(key_).toString();
        settings.setValue(key_, recentFile);
        settings.setValue(key, recentFile_);
    }

    #undef makeKey
}

void MainWindow::initRecentFilesMenu()
{
    QString baseKey(KEY_RECENT_FILE);

    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
        QString key = baseKey;
        key.append(QString::number(i));
        QString recentFile = settings.value(key).toString();

        if (!recentFile.isEmpty()) {
           addRecentFileMenu(recentFile);
        }
    }
}

void MainWindow::addRecentFileMenu(const QString& filename)
{
    ui->actionNoRecentFiles->setVisible(false);
    LvkAction* action = new LvkAction(filename);
    ui->actionOpenRecent->addAction(action);
    connect(action, SIGNAL(triggered(QString)), this, SLOT(openFile_checkUnsaved(QString)));
}

void MainWindow::closeFile_checkUnsaved()
{
    if (_sprState.hasUnsavedChanges()) {
        if (saveChangesDialog() == CancelButton) {
            return;
        }
    }
    closeFile();
}

void MainWindow::closeFile()
{
    _imgId    = 0;
    _frameId  = 0;
    _aniId    = 0;
    _aframeId = 0;

    _sprState.clear();
    setCurrentFile("");

    cellChangedSignals(false);
    ui->imgTableWidget->clearContents();
    ui->imgTableWidget->setRowCount(0);
    ui->framesTableWidget->clearContents();
    ui->framesTableWidget->setRowCount(0);
    ui->aniTableWidget->clearContents();
    ui->aniTableWidget->setRowCount(0);
    ui->aframesTableWidget->clearContents();
    ui->aframesTableWidget->setRowCount(0);
    cellChangedSignals(true);

    ui->imgPreview->setPixmap(QPixmap());
    ui->framePreview->setPixmap(QPixmap());
    ui->aframePreview->setPixmap(QPixmap());
    clearPreviewAnimation();
}

void MainWindow::exportFile()
{
    if (_exportFileName.isEmpty()) {
        exportAsFile();
    } else {
        SpriteStateError err;
        if (!_sprState.exportSprite(_exportFileName, QString(), &err)) {
           infoDialog(tr("Cannot export '") + _exportFileName + "' "
                      + SpriteState::errorMessage(err));
           return;
        }
    }
}

void MainWindow::exportAsFile()
{
    static QString exportFileName = "";

    exportFileName = QFileDialog::getSaveFileName(
            this, tr("Export file"), QFileInfo(exportFileName).absolutePath(), "*.lkot *.lkob;; *.*");

    if (!exportFileName.isEmpty()) {
        SpriteStateError err;
        if (!_sprState.exportSprite(exportFileName, QString(), &err)) {
           infoDialog(tr("Cannot export '") + exportFileName + "' "
                      + SpriteState::errorMessage(err));
           return;
        }
        setCurrentExportFile(exportFileName);
    }
}

void MainWindow::setCurrentFile(const QString& filename)
{
    if (filename.isEmpty()) {
        _filename = "";
        _exportFileName = "";

        setWindowTitle(QString(APP_NAME));
    } else  {
        QFileInfo fileInfo(filename);

        _filename = fileInfo.absoluteFilePath();
        _exportFileName = "";

        setWindowTitle(QString(APP_NAME) + " - " + fileInfo.fileName());

        storeRecentFile(fileInfo.absoluteFilePath());
        // NOTE: Workaround to get the files ordered by date in the recent files menu
        // addRecentFileMenu(filename);
        ui->actionOpenRecent->clear();
        initRecentFilesMenu();

        qDebug() << "Info: changing current app dir to" << fileInfo.absolutePath();
        QDir::setCurrent(fileInfo.absolutePath());
    }
}

void MainWindow::setCurrentExportFile(const QString& exportFileName)
{
    _exportFileName = exportFileName;
}

void MainWindow::showFramesTab()
{
    ui->tabWidget->setCurrentWidget(ui->framesTab);
}

void MainWindow::showAnimationsTab()
{
    ui->tabWidget->setCurrentWidget(ui->animationsTab);
}

void MainWindow::addImageDialog()
{
    showFramesTab();

    static QString lastDir = "";

    QStringList filenames;

    filenames = QFileDialog::getOpenFileNames(
            this, tr("Add Image"), lastDir,
            "Images(*.png *.jpg *.jpeg *.xpm *.xbm *.bmp *.tif *.tiff);; *.*");

    if (filenames.size() > 0) {
        lastDir = QFileInfo(filenames[0]).absolutePath();

        for (int i = 0; i < filenames.size(); ++i) {
            addImage(InputImage(_imgId++, filenames[i]));
        }
    }
}

void MainWindow::addImage(const InputImage& image)
{
    /* State */

    QString filename(image.filename);
    
    if (filename.isEmpty()) {
        infoDialog(tr("Empty Filename"));
        return;
    }
    if (!QFileInfo(filename).exists()) {
        infoDialog(tr("File '") + filename + tr("' does not exist"));
    } else if (QImage(filename).isNull()) {
        infoDialog(filename + tr(" has an invalid image format"));
    }

    _sprState.addImage(image);

    /* UI */

    int rows = ui->imgTableWidget->rowCount();

    QTableWidgetItem* item_id       = new QTableWidgetItem(QString::number(image.id));
    QTableWidgetItem* item_filename = new QTableWidgetItem(filename);

    cellChangedSignals(false);
    ui->imgTableWidget->setRowCount(rows+1);
    ui->imgTableWidget->setItem(rows, ColImageId, item_id);
    ui->imgTableWidget->setItem(rows, ColImageFilename, item_filename);
    ui->imgTableWidget->setCurrentItem(item_id);
    cellChangedSignals(true);

    showImage(image.id);
}

void MainWindow::showSelImage(int row)
{
    Id imgId = (row == -1) ? NullId : getImageId(row);
    showImage(imgId);
}

void MainWindow::showImage(Id imgId)
{
    const QPixmap& selPixmap = _sprState.ipixmap(imgId);
    ui->imgPreview->setPixmap(selPixmap);
}

void MainWindow::showSelImageWithFrameRect(int row, const QRect& rect)
{
    showSelImage(row);
    ui->imgPreview->setFrameRect(rect);
}

void MainWindow::removeSelImage()
{
    showFramesTab();

    int currentRow = ui->imgTableWidget->currentRow();

    if (currentRow == -1) {
        infoDialog(tr("No image selected"));
        return;
    }

    QString imgFilename = ui->imgTableWidget->item(currentRow, ColImageFilename)->text();
    if (!yesNoDialog(tr("Are you sure you want to remove the image '") + imgFilename  + tr("'?"))) {
        return;
    }

    removeImage(currentRow);

    ui->imgPreview->setPixmap(QPixmap());
}

void MainWindow::removeImage(int row)
{
    Id imgId = getImageId(row);
    qDebug() << ui->imgTableWidget->item(row, ColImageId)->text();
    qDebug() << ui->imgTableWidget->item(row, ColImageId)->text().toInt();

    cellChangedSignals(false);
    ui->imgTableWidget->removeRow(row);
    cellChangedSignals(true);

    _sprState.removeImage(imgId);

    /* remove frames that use this image*/
    for (int r = 0; r < ui->framesTableWidget->rowCount(); ++r) {
        qDebug() << "row " << r << " -- frameImgId " << getFrameImgId(r) << " == imgId " << imgId;
        if (getFrameImgId(r) == imgId) {
            removeFrame(r);
        }
    }
}

bool MainWindow::addFrameDialog()
{
    showFramesTab();

    if (ui->imgTableWidget->currentRow() == -1) {
        infoDialog(tr("No input image selected"));
        return false;
    }

    bool ok;
    QString name = QInputDialog::getText(this, tr("New frame"),
                                         tr("Frame name:"),
                                         QLineEdit::Normal, "", &ok);
    if (!ok) {
        return false;
    }

    name = name.trimmed();
    if (name.isEmpty())  {
        infoDialog(tr("Cannot add a frame without name"));
        return false;
    } else if (name.contains(",")) {
        infoDialog(tr("Frame name cannot contain the character ','"));
        return false;
    }

    Id imgId = selectedImgId();

    int ox;
    int oy;
    int w;
    int h;

    QRect frameRect = ui->imgPreview->mouseFrameRect();
    bool validRect = true;

    if (frameRect.isNull()) {
        /* add frame using the whole image */
        ox = 0;
        oy = 0;
        w  = _sprState.ipixmap(imgId).width();
        h  = _sprState.ipixmap(imgId).height();
    } else if (frameRect.width() == 0) {
        infoDialog(tr("Cannot add a frame with null width"));
        validRect = false;
    } else if (frameRect.height() == 0) {
        infoDialog(tr("Cannot add a frame with null height"));
        validRect = false;
    } else {
        /* add frame from selection in the input image */
        ox = frameRect.x();
        oy = frameRect.y();
        w  = frameRect.width();
        h  = frameRect.height();
    }

    if (validRect) {
        addFrame(LvkFrame(_frameId++, imgId, ox, oy, w, h, name));
        return true;
    }
    return false;
}

void MainWindow::addFrame(const LvkFrame &frame)
{
    /* state */

    _sprState.addFrame(frame);

    /* UI */

    QTableWidgetItem* item_id   = new QTableWidgetItem(QString::number(frame.id));
    QTableWidgetItem* item_ox   = new QTableWidgetItem(QString::number(frame.ox));
    QTableWidgetItem* item_oy   = new QTableWidgetItem(QString::number(frame.oy));
    QTableWidgetItem* item_w    = new QTableWidgetItem(QString::number(frame.w));
    QTableWidgetItem* item_h    = new QTableWidgetItem(QString::number(frame.h));
    QTableWidgetItem* item_iid  = new QTableWidgetItem(QString::number(frame.imgId));
    QTableWidgetItem* item_name = new QTableWidgetItem(frame.name);

    int rows = ui->framesTableWidget->rowCount();

    cellChangedSignals(false);
    ui->framesTableWidget->setRowCount(rows+1);
    ui->framesTableWidget->setItem(rows, ColFrameId,    item_id);
    ui->framesTableWidget->setItem(rows, ColFrameOx,    item_ox);
    ui->framesTableWidget->setItem(rows, ColFrameOy,    item_oy);
    ui->framesTableWidget->setItem(rows, ColFrameW,     item_w);
    ui->framesTableWidget->setItem(rows, ColFrameH,     item_h);
    ui->framesTableWidget->setItem(rows, ColFrameImgId, item_iid);
    ui->framesTableWidget->setItem(rows, ColFrameName,  item_name);
    cellChangedSignals(true);

    ui->imgTableWidget->setCurrentItem(item_id);

    showFrame(frame.id);
}

void MainWindow::showSelFrame(int row)
{
    Id frameId = (row == -1) ? NullId : getFrameId(row);
    showFrame(frameId);
}

void MainWindow::showFrame(Id frameId)
{
    /* show frame preview */
    const QPixmap& selPixmap = _sprState.fpixmap(frameId);
    ui->framePreview->setPixmap(selPixmap);

    /* show frame rect in the input image*/
    if (frameId == NullId) {
        showSelImageWithFrameRect(-1, QRect());
    } else {
        const LvkFrame frame = _sprState.const_frame(frameId);
        for (int r = 0; r < ui->imgTableWidget->rowCount(); r++) {
            if (frame.imgId == getImageId(r)) {
                ui->imgTableWidget->selectRow(r);
                showSelImageWithFrameRect(r, frame.rect());
                break;
            }
        }
    }
}

void MainWindow::removeSelFrame()
{
    showFramesTab();

    int currentRow = ui->framesTableWidget->currentRow();

    if (currentRow == -1) {
        infoDialog(tr("No frame selected"));
        return;
    }

    QString frameName = ui->framesTableWidget->item(currentRow, ColFrameName)->text();
    if (!yesNoDialog(tr("Are you sure you want to remove the frame '") + frameName  + tr("'?"))) {
        return;
    }

    removeFrame(currentRow);
}

void MainWindow::removeFrame(int row)
{
    Id frameId = getFrameId(row);

    cellChangedSignals(false);
    ui->framesTableWidget->removeRow(row);
    cellChangedSignals(true);

    ui->framePreview->setPixmap(QPixmap());

    _sprState.removeFrame(frameId);
}

void MainWindow::addAnimationDialog()
{
    showAnimationsTab();

    bool ok;
    QString name = QInputDialog::getText(this, tr("New animation"),
                                         tr("Animation name:"),
                                         QLineEdit::Normal, "", &ok);
    if (ok) {
        name = name.trimmed();
        if (name.isEmpty())  {
            infoDialog(tr("Cannot add an animation without name"));
            return;
        }
        addAnimation(LvkAnimation(_aniId++, name));
    }
}

void MainWindow::hideFramePreview()
{
    ui->hSpacerFramePreview->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
    ui->frameZoomInButton->hide();
    ui->frameZoomOutButton->hide();
    ui->framePreviewScroll->hide();
    ui->hideFramePreviewButton->setIcon(QIcon(":/buttons/button-show"));
    ui->hideFramePreviewButton->setToolTip(tr("Show frames preview"));
    ui->framePreviewLayout->update();
}

void MainWindow::showFramePreview()
{
    ui->hSpacerFramePreview->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->frameZoomInButton->show();
    ui->frameZoomOutButton->show();
    ui->framePreviewScroll->show();
    ui->hideFramePreviewButton->setIcon(QIcon(":/buttons/button-hide"));
    ui->hideFramePreviewButton->setToolTip(tr("Hide frames preview"));
    ui->framePreviewLayout->update();
}

void MainWindow::hideShowFramePreview()
{
    static bool visible = false;

    if (visible) {
        hideFramePreview();
    } else {
        showFramePreview();
    }
    visible = !visible;
}

void MainWindow::addAnimation(const LvkAnimation& ani)
{
    /* state */

    _sprState.addAnimation(ani);

    /* UI */

    int rows = ui->aniTableWidget->rowCount();

    QTableWidgetItem* item_id   = new QTableWidgetItem(QString::number(ani.id));
    QTableWidgetItem* item_name = new QTableWidgetItem(ani.name);

    cellChangedSignals(false);
    ui->aniTableWidget->setRowCount(rows+1);
    ui->aniTableWidget->setItem(rows, ColAniId, item_id);
    ui->aniTableWidget->setItem(rows, ColAniName, item_name);
    ui->aniTableWidget->setCurrentItem(item_id);
    cellChangedSignals(true);

    showAframes(rows);
    clearPreviewAnimation();
}

void MainWindow::showAframes(int row)
{
    if (currentAnimation && currentAnimation->isAnimated()) {
        currentAnimation->stopAnimation();
    }

    cellChangedSignals(false);

    ui->aframesTableWidget->clearContents();
    ui->aframesTableWidget->setRowCount(0);

    if (row == -1) {
        return;
    }

    int animationId = getAnimationId(row);
    QList<QGraphicsPixmapItem*> aniFrames;
    LvkAnimation ani = _sprState.animations().value(animationId);
    for (QHashIterator<Id, LvkAframe> it(ani.aframes); it.hasNext();){
        LvkAframe aFrame = it.next().value();
        addAframe_(aFrame,animationId);
    }

    if (ui->aframesTableWidget->rowCount() > 0) {
        ui->aframesTableWidget->selectRow(0);
        showSelAframe(0);
    }

    cellChangedSignals(true);

    previewAnimation(); /* automatic preview */

}

void MainWindow::previewAnimation()
{
    if (ui->aniTableWidget->currentRow() == -1) {
        infoDialog(tr("No animation selected"));
        return;
    }

    // TODO optimize this! If the animation did not change, do not delete and recreate
    // the animation

    static QGraphicsScene* scene = new QGraphicsScene;

    if (currentAnimation) {
        scene->removeItem(currentAnimation);
        delete currentAnimation;
    }
    LvkAnimation selectedAni = _sprState.animations().value(selectedAniId());
    currentAnimation = new LvkFrameGraphicsGroup(selectedAni, _sprState.fpixmaps());
    scene->addItem(currentAnimation);
    ui->aniPreview->setScene(scene);
    currentAnimation->startAnimation();
}

void MainWindow::clearPreviewAnimation()
{
    if (ui->aniPreview->scene() && currentAnimation) {
        ui->aniPreview->scene()->removeItem(currentAnimation);
    }
}

void MainWindow::removeSelAnimation()
{
    showAnimationsTab();

    int currentRow = ui->aniTableWidget->currentRow();

    if (currentRow == -1) {
        infoDialog(tr("No animation selected"));
        return;
    }

    QString aniName = ui->aniTableWidget->item(currentRow, ColAniName)->text();
    if (!yesNoDialog(tr("Are you sure you want to remove the animation '") + aniName  + tr("'?"))) {
        return;
    }

    removeAnimation(currentRow);

    showAframes(ui->aniTableWidget->currentRow());
}

void MainWindow::removeAnimation(int row)
{
    Id aniId = getAnimationId(row);

    cellChangedSignals(false);
    ui->aniTableWidget->removeRow(row);
    cellChangedSignals(true);

    clearPreviewAnimation();

    _sprState.removeAnimation(aniId);
}

void MainWindow::addAframeDialog()
{
    if (_sprState.frames().isEmpty()) {
        infoDialog(tr("No frames available.\n\nGo to the \"Frames\" tab and create at least one frame."));
        return;
    }
    if (ui->aniTableWidget->currentRow() == -1) {
        infoDialog(tr("No animation selected"));
        return;
    }

    QStringList framesList;

    for (QHashIterator<int, LvkFrame> it(_sprState.frames()); it.hasNext();) {
        it.next();
        const LvkFrame& frame = it.value();
        framesList << tr("Id: ") + QString::number(frame.id) +  tr(" Name: ") + frame.name;
        framesList.sort();
    }

    bool ok;
    QString frame_str = QInputDialog::getItem(this, tr("New Animation frame"),
                                          tr("Choose a frame:"),
                                          framesList, 0, false, &ok);
    if (ok) {
        QStringList tokens = frame_str.split(" ");

        if (tokens.size() < 2) {
            infoDialog(tr("Cannot add frame. The selected frame could not be parsed"));
            return;
        }
        Id frameId = tokens.at(1).toInt();

        addAframe(LvkAframe(_aframeId++, frameId), selectedAniId());
    }
}


void MainWindow::addAframe(const LvkAframe& aframe, Id aniId)
{
    /* state */
    _sprState.addAframe(aframe, aniId);
    
    /* UI */
    addAframe_(aframe, aniId);
}

void MainWindow::addAframe_(const LvkAframe& aframe, Id aniId)
{
    QTableWidgetItem* item_id    = new QTableWidgetItem(QString::number(aframe.id));
    QTableWidgetItem* item_fid   = new QTableWidgetItem(QString::number(aframe.frameId));
    QTableWidgetItem* item_delay = new QTableWidgetItem(QString::number(aframe.delay));
    QTableWidgetItem* item_ox    = new QTableWidgetItem(QString::number(aframe.ox));
    QTableWidgetItem* item_oy    = new QTableWidgetItem(QString::number(aframe.oy));
    QTableWidgetItem* item_aniId = new QTableWidgetItem(QString::number(aniId));

    int rows = ui->aframesTableWidget->rowCount();

    cellChangedSignals(false);
    ui->aframesTableWidget->setRowCount(rows+1);
    ui->aframesTableWidget->setItem(rows, ColAframeId,      item_id);
    ui->aframesTableWidget->setItem(rows, ColAframeFrameId, item_fid);
    ui->aframesTableWidget->setItem(rows, ColAframeDelay,   item_delay);
    ui->aframesTableWidget->setItem(rows, ColAframeOx,      item_ox);
    ui->aframesTableWidget->setItem(rows, ColAframeOy,      item_oy);
    ui->aframesTableWidget->setItem(rows, ColAframeAniId,   item_aniId);
    ui->aframesTableWidget->setCurrentItem(item_id);
    cellChangedSignals(true);

    showAframe(aframe.id);

    previewAnimation();
}


void MainWindow::showSelAframe(int row)
{
    Id frameId = (row == -1) ? NullId : getAFrameFrameId(row);
    showAframe(frameId);
}

void MainWindow::showAframe(Id frameId)
{
    const QPixmap& selPixmap = _sprState.fpixmap(frameId);
    int w = selPixmap.width();
    int h = selPixmap.height();

    ui->aframePreview->setPixmap(selPixmap);
    ui->aframePreview->setGeometry(0, 0, w, h);
    ui->aframePreview->updateGeometry();
}

void MainWindow::removeSelAframe()
{
    showAnimationsTab();

    int currentRow = ui->aframesTableWidget->currentRow();

    if (currentRow == -1) {
        infoDialog(tr("No frame selected"));
        return;
    }

    if (!yesNoDialog(tr("Are you sure you want to remove the selected frame?"))) {
        return;
    }

    removeAframe(currentRow);
}

void MainWindow::removeAframe(int row)
{
    Id frameId = getAFrameId(row);

    cellChangedSignals(false);
    ui->aframesTableWidget->removeRow(row);
    cellChangedSignals(true);

    ui->aframePreview->setPixmap(QPixmap());

    _sprState.removeAframe(frameId, selectedAniId());

    previewAnimation();
}

void MainWindow::incAniSpeed(int ms)
{
    if (ui->aniTableWidget->currentRow() == -1) {
        infoDialog(tr("No animation selected"));
        return;
    }

    Id aniId = selectedAniId();

    for (int r = 0; r < ui->aframesTableWidget->rowCount(); ++r) {
        int& delay = _sprState.aframe(aniId, getAFrameId(r)).delay;
        delay -= ms;
        if (delay < 0) {
            delay = 0;
        }
        ui->aframesTableWidget->item(r, ColAframeDelay)->setText(QString::number(delay));
    }
    previewAnimation();
}

void MainWindow::decAniSpeed(int ms)
{
    incAniSpeed(-ms);
}

void MainWindow::updateImgTable(int row, int col)
{
    QTableWidget* table    = ui->imgTableWidget;
    QString       newValue = getItem(table, row, col);
    Id            imgId    = getIdItem(table, row, ColImageId);
    InputImage&   img      = _sprState.image(imgId);

    switch (col) {
    case ColImageId:
        infoDialog(tr("Column \"Id\" is not editable"));
        setItem(table, row, col, imgId);
        break;
    case ColImageFilename:
        if (newValue.isEmpty()) {
            infoDialog(tr("Image filename cannot be empty"));
            setItem(table, row, col, img.filename);
        } else if (newValue.contains(',')) {
            infoDialog(tr("Image filename cannot contain the character ','"));
            setItem(table, row, col, img.filename);
        } else if (newValue != img.filename) {
            img.filename = newValue;
            img.reloadImage();
            if (!QFileInfo(newValue).exists()) {
                infoDialog(tr("The file does not exist"));
            } else if (img.pixmap.isNull()) {
                infoDialog(tr("The file contains an invalid image format"));
            }
            _sprState.reloadFramePixmaps(img);

            /* update UI */

            setItem(table, row, col, img.filename);

            showSelImage(row);
            if (ui->framesTableWidget->currentRow() != -1) {
                showSelFrame(ui->framesTableWidget->currentRow());
            }
            if (ui->aframesTableWidget->currentRow() != -1) {
                showSelAframe(ui->aframesTableWidget->currentRow());
            }
            previewAnimation();
        }
        break;
    }
}

void MainWindow::updateFramesTable(int row, int col)
{
    QTableWidget* table    = ui->framesTableWidget;
    QString       newValue = getItem(table, row, col);
    Id            frameId  = getIdItem(table, row, ColFrameId);
    LvkFrame&     frame    = _sprState.frame(frameId);

    bool ok = true;
    int i = newValue.toInt(&ok);

    switch (col) {
    case ColFrameId:
        ok = false;
        infoDialog(tr("Column \"Id\" is not editable"));
        setItem(table, row, col, frameId);
        break;
    case ColFrameImgId:
        ok = false;
        infoDialog(tr("Column \"Image Id\" is not editable."));
        setItem(table, row, col, frame.imgId);
        break;
    case ColFrameOx:
        if (ok) {
            frame.ox = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, frame.ox);
        }
        break;
    case ColFrameOy:
        if (ok) {
            frame.oy = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, frame.oy);
        }
        break;
    case ColFrameW:
        if (ok) {
            frame.w = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, frame.w);
        }
        break;
    case ColFrameH:
        if (ok) {
            frame.h = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, frame.h);
        }
        break;
    case ColFrameName:
        if (newValue.isEmpty()) {
            ok = false;
            infoDialog(tr("Frame name cannot be empty"));
        } else if (newValue.contains(',')) {
            ok = false;
            infoDialog(tr("Frame name cannot contain the character ','"));
        }
        if (ok) {
            frame.name = newValue;
        } else {
            setItem(table, row, col, frame.name);
        }
        break;
    }

    if (ok) {
        switch (col) {
        case ColFrameOx:
        case ColFrameOy:
        case ColFrameW:
        case ColFrameH:
            _sprState.reloadFramePixmap(frame);
            ui->imgPreview->setFrameRect(frame.rect());
            showFrame(frame.id);
            break;
        default:
            break;
        }
    }
}

void MainWindow::updateAframesTable(int row, int col)
{
    QTableWidget* table    = ui->aframesTableWidget;
    QString       newValue = getItem(table, row, col);
    Id            aniId    = getIdItem(table, row, ColAframeAniId);
    Id            aframeId = getIdItem(table, row, ColAframeId);
    LvkAframe&    aframe   = _sprState.aframe(aniId, aframeId);

    bool ok = true;
    int i = newValue.toInt(&ok);

    switch (col) {
    case ColAframeId:
        ok = false;
        infoDialog(tr("Column \"Id\" is not editable"));
        setItem(table, row, col, aframeId);
        break;
    case ColAframeAniId:
        ok = false;
        infoDialog(tr("Column \"Animation Id\" is not editable."));
        setItem(table, row, col, aniId);
        break;
    case ColAframeFrameId:
        if (ok) {
            aframe.frameId = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, aframe.frameId);
        }
        break;
    case ColAframeOx:
        if (ok) {
            aframe.ox = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, aframe.ox);
        }
        break;
    case ColAframeOy:
        if (ok) {
            aframe.oy = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, aframe.oy);
        }
        break;
    case ColAframeDelay:
        if (ok) {
            aframe.delay = i;
        } else {
            infoDialog(tr("Invalid input."));
            setItem(table, row, col, aframe.delay);
        }
        break;
    }

    if (ok && currentAnimation->isAnimated()) {
        previewAnimation(); /* force refresh animation */
    }
}

void MainWindow::updateAniTable(int row, int col)
{
    QTableWidget* table    = ui->aniTableWidget;
    QString       newValue = getItem(table, row, col);
    Id            aniId    = getIdItem(table, row, ColAniId);
    LvkAnimation& ani      = _sprState.animation(aniId);

    switch (col) {
    case ColAniId:
        infoDialog(tr("Column \"Id\" is not editable"));
        setItem(table, row, col, aniId);
        break;
    case ColAniName:
        if (newValue.isEmpty()) {
            infoDialog(tr("Animation name cannot be empty"));
            setItem(table, row, col, ani.name);
        } else if (newValue.contains(',')) {
            infoDialog(tr("Animation name cannot contain the character ','"));
            setItem(table, row, col, ani.name);
        } else {
            ani.name = newValue;
        }
        break;
    }
}

void MainWindow::showMousePosition(int x, int y)
{
    statusBarMousePos->setText(tr("Mouse x,y: ") + QString::number(x) + "," + QString::number(y));
}

void MainWindow::showMouseRect(const QRect& rect)
{
    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();

    if (w == 0 && h == 0) {
        statusBarRectSize->setText("");
    } else {
        statusBarRectSize->setText(tr("  Rect: x,y,w,h: ") +
                                   QString::number(x) + "," + QString::number(y) + "," +
                                   QString::number(w) + "," + QString::number(h));
    }
}

void MainWindow::undo()
{
    if (_sprState.canUndo()) {
        infoDialog(tr("Could not undo"));
    }
}

void MainWindow::redo()
{
    if (_sprState.canRedo()) {
        infoDialog(tr("Could not redo"));
    }
}

void MainWindow::about()
{
    infoDialog(QString(APP_NAME)  + " " + QString(APP_VERSION));
}

void MainWindow::exit()
{
    if (_sprState.hasUnsavedChanges()) {
        if (saveChangesDialog() == CancelButton) {
            return;
        }
    }
    QCoreApplication::exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    exit();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        ui->imgPreview->update();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        ui->imgPreview->update();
    }
}

MainWindow::~MainWindow()
{
    delete statusBarRectSize;
    delete statusBarMousePos;
    delete ui;
}




