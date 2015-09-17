#ifndef METADATAPARSER_H
#define METADATAPARSER_H

#include <QtGlobal>
#include <QString>
#include <QVector2D>
#include <QVector3D>
#include <QSizeF>
#include <QList>
#include <QMap>

#include "../LibHVR/cpp/playbackmetadata.h"

#ifdef Q_OS_WIN
#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
#endif

class MetadataParser
{
public:
    MetadataParser();
    ~MetadataParser();

    // v. 0.1 "network" metadata hack
    void setMetadataString(QString metadata);
    QString metadataString() { return m_metadata; }

    // v. 0.2
    bool parseMetadataPackage(u_int8_t *data, size_t size);

    QString lensMappingEnum() const { return m_lensMappingEnum; }
    QString versionString() const { return m_versionString; }
    double lensFov() { return m_lensFov; }
    QMap<u_int8_t, ImageSourceMetadata0> imageSourceMetadata() { return m_imageSourceMetadata; }
    QMap<u_int8_t, AudioSourceMetadata0> audioSourceMetadata() { return m_audioSourceMetadata; }

    QString humanReadableString();

private:
    QString m_metadata;

    QString m_versionString;
    int8_t m_thumbnailStreamIndex;
    QString m_lensMappingEnum;
    int8_t m_lensMappingStreamIndex;
    double m_lensFov;
    int8_t m_orientationStreamIndex;
    int16_t m_keyframeInterval;

    QMap<u_int8_t, ImageSourceMetadata0> m_imageSourceMetadata; // key == sourceId
    QMap<u_int8_t, AudioSourceMetadata0> m_audioSourceMetadata; // key == sourceId
};

#endif // METADATAPARSER_H
