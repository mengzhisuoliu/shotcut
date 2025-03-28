/*
 * Copyright (c) 2013-2024 Meltytech, LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILTER_H
#define FILTER_H

#include "qmlmetadata.h"
#include "shotcut_mlt_properties.h"

#include <MltAnimation.h>
#include <MltProducer.h>
#include <MltService.h>
#include <QColor>
#include <QObject>
#include <QRectF>
#include <QString>
#include <QUuid>
#include <QVariant>

class AbstractJob;
class EncodeJob;
class QUndoCommand;
class FilterController;

class QmlFilter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isNew READ isNew CONSTANT)
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(QStringList presets READ presets NOTIFY presetsChanged)
    Q_PROPERTY(int in READ in NOTIFY inChanged)
    Q_PROPERTY(int out READ out NOTIFY outChanged)
    Q_PROPERTY(int animateIn READ animateIn WRITE setAnimateIn NOTIFY animateInChanged)
    Q_PROPERTY(int animateOut READ animateOut WRITE setAnimateOut NOTIFY animateOutChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(bool blockSignals READ signalsBlocked WRITE blockSignals)

public:
    enum CurrentFilterIndex { NoCurrentFilter = -1, DeselectCurrentFilter = -2 };
    Q_ENUM(CurrentFilterIndex)

    explicit QmlFilter();
    explicit QmlFilter(Mlt::Service &mltService,
                       const QmlMetadata *metadata,
                       QObject *parent = nullptr);
    ~QmlFilter();

    bool isNew() const { return m_isNew; }
    void setIsNew(bool isNew) { m_isNew = isNew; }

    Q_INVOKABLE QString get(QString name, int position = -1);
    Q_INVOKABLE QColor getColor(QString name, int position = -1);
    Q_INVOKABLE double getDouble(QString name, int position = -1);
    Q_INVOKABLE QRectF getRect(QString name, int position = -1);
    Q_INVOKABLE void removeRectPercents(QString name);
    Q_INVOKABLE QStringList getGradient(QString name);
    Q_INVOKABLE void set(QString name, QString value, int position = -1);
    Q_INVOKABLE void set(QString name,
                         const QColor &value,
                         int position = -1,
                         mlt_keyframe_type keyframeType = mlt_keyframe_type(-1));
    Q_INVOKABLE void set(QString name,
                         double value,
                         int position = -1,
                         mlt_keyframe_type keyframeType = mlt_keyframe_type(-1));
    Q_INVOKABLE void set(QString name,
                         int value,
                         int position = -1,
                         mlt_keyframe_type keyframeType = mlt_keyframe_type(-1));
    Q_INVOKABLE void set(QString name,
                         bool value,
                         int position = -1,
                         mlt_keyframe_type keyframeType = mlt_keyframe_type(-1));
    Q_INVOKABLE void set(QString name,
                         double x,
                         double y,
                         double width,
                         double height,
                         double opacity = 1.0,
                         int position = -1,
                         mlt_keyframe_type keyframeType = mlt_keyframe_type(-1));
    Q_INVOKABLE void set(QString name,
                         const QRectF &rect,
                         int position = -1,
                         mlt_keyframe_type keyframeType = mlt_keyframe_type(-1));
    Q_INVOKABLE void setGradient(QString name, const QStringList &gradient);
    QString path() const { return m_path; }
    Q_INVOKABLE void loadPresets();
    QStringList presets() const { return m_presets; }
    /// returns the index of the new preset
    Q_INVOKABLE int savePreset(const QStringList &propertyNames, const QString &name = QString());
    Q_INVOKABLE void deletePreset(const QString &name);
    Q_INVOKABLE void analyze(bool isAudio = false, bool deferJob = true);
    Q_INVOKABLE static int framesFromTime(const QString &time);
    Q_INVOKABLE void getHash();
    Mlt::Producer &producer() { return m_producer; }
    int in();
    int out();
    Mlt::Service &service() { return m_service; }
    int animateIn();
    void setAnimateIn(int value);
    int animateOut();
    void setAnimateOut(int value);
    void clearAnimateInOut();
    int duration();
    Q_INVOKABLE void resetProperty(const QString &name);
    Q_INVOKABLE void clearSimpleAnimation(const QString &name);
    Mlt::Animation getAnimation(const QString &name);
    Q_INVOKABLE int keyframeCount(const QString &name);
    mlt_keyframe_type getKeyframeType(Mlt::Animation &animation,
                                      int position,
                                      mlt_keyframe_type defaultType);
    Q_INVOKABLE int getKeyFrameType(const QString &name, int keyIndex);
    Q_INVOKABLE void setKeyFrameType(const QString &name, int keyIndex, int type);
    Q_INVOKABLE int getNextKeyframePosition(const QString &name, int position);
    Q_INVOKABLE int getPrevKeyframePosition(const QString &name, int position);
    Q_INVOKABLE bool isAtLeastVersion(const QString &version);
    Q_INVOKABLE static void deselect();
    bool allowTrim() const;
    bool allowAnimateIn() const;
    bool allowAnimateOut() const;
    Q_INVOKABLE void crop(const QRectF &rect);
    QString objectNameOrService();

    Q_INVOKABLE void copyParameters();
    Q_INVOKABLE void pasteParameters(const QStringList &propertyNames);

    // Functions for undo/redo
    void startUndoTracking();
    void stopUndoTracking();
    Q_INVOKABLE void startUndoParameterCommand(const QString &desc = QString());
    void startUndoAddKeyframeCommand();
    void startUndoRemoveKeyframeCommand();
    void startUndoModifyKeyframeCommand(int paramIndex, int keyframeIndex);
    void updateUndoCommand(const QString &name);
    Q_INVOKABLE void endUndoCommand();

public slots:
    void preset(const QString &name);

signals:
    void presetsChanged();
    void analyzeFinished(bool isSuccess);
    void changed(QString name = QString());
    void inChanged(int delta);
    void outChanged(int delta);
    void animateInChanged();
    void animateOutChanged();
    void animateInOutChanged();
    void durationChanged();
    void propertyChanged(QString name); // Use to let QML know when a specific property has changed

private:
    const QmlMetadata *m_metadata;
    Mlt::Service m_service;
    Mlt::Producer m_producer;
    QString m_path;
    bool m_isNew;
    QStringList m_presets;
    Mlt::Properties m_previousState;
    int m_changeInProgress;

    int keyframeIndex(Mlt::Animation &animation, int position);
};

class AnalyzeDelegate : public QObject
{
    Q_OBJECT
public:
    explicit AnalyzeDelegate(Mlt::Filter &filter);

public slots:
    void onAnalyzeFinished(AbstractJob *job, bool isSuccess);

private:
    QString resultsFromXml(const QString &fileName);
    void updateFilter(Mlt::Filter &filter, const QString &results);
    void updateJob(EncodeJob *job, const QString &results);

    QUuid m_uuid;
};

#endif // FILTER_H
