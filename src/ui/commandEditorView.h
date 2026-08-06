#pragma once

#include <QtWidgets>
#include <QString>

#include "document/config.h"
#include "bin/timing.h"
#include "context.h"
#include "logViewer.h"
#include "baseTreeView.h"
#include "concordConnection.h"

namespace editors
{

/**
 * @brief list tree view for raw commands
 */
class RawCommandTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit RawCommandTreeView(Context &ctx, QWidget *parent = nullptr);
    ~RawCommandTreeView() override;

    void setModel(QAbstractItemModel *model) override;

  private:
    void setupDelegates();
};

/**
 * @brief list tree view for protocol commands
 */
class ProtoCommandTreeView: public BaseTreeView
{
  Q_OBJECT

  public:
    explicit ProtoCommandTreeView(Context &ctx, QWidget *parent = nullptr);
    ~ProtoCommandTreeView() override;

    void setModel(QAbstractItemModel *model) override;

  private:
    void setupDelegates();
};


/**
 * @brief editor view for ir command editing (raw, proto, common params)
 */
class CommandEditorView: public QWidget
{
  Q_OBJECT

  public:
    explicit CommandEditorView(Context &ctx, QWidget *parent = nullptr);
    ~CommandEditorView();

    /** add tree views to the command editor */
    void addTreeViews(ProtoCommandTreeView *protoTree, RawCommandTreeView *rawTree);

    /** load data, update models
     *
     * non-table values are directly pulled / set from the config data set, not via model */
    void setData(uint32_t deviceId, QAbstractItemModel *protoModel, QAbstractItemModel *rawModel);

    void setLearnedCommand(ConcordConnection::LearnedCommandMode m, const binary::TimingStream &t, uint32_t carrier);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    /** view is ready to process a learned IR command */
    void enableLernMode(bool start);

  protected slots:
    void onEditingPressPreSilenceFinished();
    void onEditingHoldPreSilenceFinished();
    void onEditingInterKeyFinished();
    void onIrDeviceDataChanged(document::data::Item item, uint32_t pos);

  protected:
    void createView();
    void setupTreeView();
    void createConnections();

  protected:
    Context &ctx;
    uint32_t deviceId = 0;

  private:
    QVBoxLayout *layout = nullptr;
    QSplitter *splitter = nullptr;
    QLabel *header = nullptr;

    QSpinBox *pressPreSilenceSpinBox;
    QSpinBox *holdPreSilencSpinBox;
    QSpinBox *interKeySpinBox; //combines both inter-key from the data set
};


} // namespace editors
