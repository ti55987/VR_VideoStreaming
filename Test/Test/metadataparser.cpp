#include "metadataparser.h"

#include <QStringList>
#include <QDebug>

MetadataParser::MetadataParser()
    : m_thumbnailStreamIndex(-1)
    , m_lensMappingEnum("")
    , m_lensMappingStreamIndex(-1)
    , m_lensFov(0)
    , m_orientationStreamIndex(-1)
    , m_keyframeInterval(-1)
{
}

MetadataParser::~MetadataParser()
{
}

void MetadataParser::setMetadataString(QString metadata)
{
    if (m_metadata == metadata)
        return;

    m_metadata = metadata;

    if( metadata.contains(QString("NokiaPC"))) {
        qDebug() << "Likely a valid file:" << m_metadata;
        if( metadata.contains(QString("network="))) {
            qDebug() << "Stripping extra network prefix";
            m_metadata.remove(0, metadata.indexOf(QString("network="))+8);
            m_metadata.replace(QString("\\"), QString(""));
            qDebug() << "New metadata: " << m_metadata;
        }
    }

    QStringList lines = m_metadata.split('\n');

    // identifier
    // version
    // thumbnail stream
    // lens mapping enum
    // lens mapping stream
    // lens fov
    // orientation stream
    // keyframe interval
    // video streams
    // audio streams
    if (lines[0] != QStringLiteral("NokiaPC")) {
        qWarning() << "Not a compatible Nokia Presence Capture file!";
        return;
    }
    if (lines[1] != QStringLiteral("0.1")) {
        if (lines[1] == QStringLiteral("0.2")) {
            qDebug() << "File format version 0.2, metadata should be in a separate stream..";
            m_versionString = lines[1];
            return;
        }
        qWarning() << "Unsupported version " << lines[1];
        return;
    }
    m_versionString = lines[1];
    m_thumbnailStreamIndex = lines[2].toInt();
    m_lensMappingEnum = lines[3];
    m_lensMappingStreamIndex = lines[4].toInt();
    m_lensFov = lines[5].toDouble();
    m_orientationStreamIndex = lines[6].toInt();
    m_keyframeInterval = lines[7].toInt();
    m_imageSourceMetadata.clear();

    // Video streams
    QStringList videoStreams = lines[8].split(':');
    int count = 0;
    foreach(QString videoStream, videoStreams) {
        QStringList data = videoStream.split(',');
        qDebug() << data;

        ImageSourceMetadata0 sourceMetadata;
        u_int8_t streamIndex = data[0].toInt();

        sourceMetadata.extrinsicYaw = data[4].toDouble();
        sourceMetadata.extrinsicPitch = -data[5].toDouble();
        sourceMetadata.extrinsicRoll = -data[6].toDouble();
        sourceMetadata.extrinsicX = data[7].toDouble();
        sourceMetadata.extrinsicY = data[8].toDouble();
        sourceMetadata.extrinsicZ = data[9].toDouble();
        sourceMetadata.lensFov = m_lensFov;

        if (m_lensMappingEnum == "SunexDSL315") {
            sourceMetadata.lensType = ImageSourceMetadata0::PresenceCaptureDevice;
            sourceMetadata.lensSubType = 0; // ImagingLensTypeDSL315 in sdidata.h
        }

        sourceMetadata.sourceId = count;
        count++;
        QVector2D centerPoint(data[1].toDouble(), data[2].toDouble());
        double diameter = data[3].toDouble();
        sourceMetadata.sourceX = (centerPoint.x() - diameter * 0.5);
        sourceMetadata.sourceY = (centerPoint.y() - diameter * 0.5);
        sourceMetadata.sourceWidth = diameter;
        sourceMetadata.sourceHeight = diameter;
        sourceMetadata.streamIndex = streamIndex;
        m_imageSourceMetadata[sourceMetadata.sourceId] = sourceMetadata;
    }

    // Audio streams
    QStringList audioStreams = lines[9].split(':', QString::SkipEmptyParts);
    m_audioSourceMetadata.clear();

    int channel = 0;
    foreach(QString audioStream, audioStreams) {
        QStringList data = audioStream.split(',');
        u_int8_t streamIndex = data[0].toInt();

        AudioSourceMetadata0 metadata;
        metadata.streamIndex = streamIndex;
        metadata.positionX = data[1].toDouble();
        metadata.positionY = data[2].toDouble();
        metadata.positionZ = data[3].toDouble();
        metadata.signalType = AudioSourceMetadata0::SignalSpeaker;
        metadata.sourceId = channel;
        channel++;
        m_audioSourceMetadata[metadata.sourceId] = metadata;
    }
}

bool MetadataParser::parseMetadataPackage(u_int8_t *data, size_t size)
{
    size_t read = 0;
    while (read < size) {
        PlaybackMetadataType type = (PlaybackMetadataType)data[read];
        qDebug() << "Reading metadata type" << type << "at" << read;
        switch(type) {
        case ImageSourceMetadata0Type: {
            ImageSourceMetadata0 *metadata = reinterpret_cast<ImageSourceMetadata0*>(&data[read]);
            qDebug() << "stream id:" << metadata->streamIndex <<
                        ", source id:" << metadata->sourceId;
            m_imageSourceMetadata[metadata->sourceId] = *metadata;
            read += sizeof(ImageSourceMetadata0);
            break;
        }

        case AudioSourceMetadata0Type: {
            AudioSourceMetadata0 *metadata = reinterpret_cast<AudioSourceMetadata0*>(&data[read]);
            qDebug() << "stream id:" << metadata->streamIndex;
            qDebug() << "source id:" << metadata->sourceId;
            m_audioSourceMetadata[metadata->sourceId] = *metadata;
            read += sizeof(AudioSourceMetadata0);
            break;
        }

        case OrientationMetadata0Type: {
            OrientationMetadata0 *metadata = reinterpret_cast<OrientationMetadata0*>(&data[read]);
            qDebug() << "orientation:" << metadata->x << "," << metadata->y << "," << metadata->z;
            read += sizeof(OrientationMetadata0);
            break;
        }

        default:
            qWarning() << "Unknown metadata type " << type;
            return false;
        }
    }

    return true;
}

QString MetadataParser::humanReadableString()
{
    QString str;
    QTextStream s(&str, QIODevice::Text);
    s << "Version: " << m_versionString << endl;
    if (m_thumbnailStreamIndex != -1)
        s << "Thumbnail stream index: " << m_thumbnailStreamIndex << endl;
    if (!m_lensMappingEnum.isEmpty())
        s << "Lens mapping enumaration: " << m_lensMappingEnum << endl;
    if (m_lensMappingStreamIndex != -1)
        s << "Lens mapping stream index: " << m_lensMappingStreamIndex << endl;
    s << "Lens FOV: " << m_lensFov << endl;
    if (m_orientationStreamIndex != -1)
        s << "Orientation stream index: " << m_orientationStreamIndex << endl;
    if (m_keyframeInterval != -1)
        s << "Key frame interval: " << m_keyframeInterval << endl;

    s << endl;
    s << "** Image sources **" << endl;

    foreach (u_int8_t sourceId, m_imageSourceMetadata.keys()) {
        ImageSourceMetadata0 &source = m_imageSourceMetadata[sourceId];

        s << "  Source id: " << sourceId << endl;
        s << "  Stream id: " << source.streamIndex << endl;
        s << "  Lens type: " << source.lensType << endl;
        s << "  Lens sub-type: " << source.lensSubType << endl;
        s << "  Lens FOV: " << source.lensFov << endl;
        s << "  Source rect: (" << source.sourceX << "," << source.sourceY << "," << source.sourceWidth << "," << source.sourceHeight << ")" << endl;
        s << "  Yaw: " << source.extrinsicYaw << endl;
        s << "  Pitch: " << source.extrinsicPitch << endl;
        s << "  Roll: " << source.extrinsicRoll << endl;
        s << "  Position: (" << source.extrinsicX << ", " << source.extrinsicY << ", " << source.extrinsicZ << ")" << endl;
        s << endl;
    }

    s << "** Audio sources **" << endl;
    foreach (u_int8_t sourceId, m_audioSourceMetadata.keys()) {
        AudioSourceMetadata0 &source = m_audioSourceMetadata[sourceId];

        s << "  Source id: " << sourceId << endl;
        s << "  Stream id: " << source.streamIndex << endl;
        s << "  Signal type: " << source.signalType;
        s << "  Position: (" << source.positionX << ", " << source.positionY << ", " << source.positionZ << ")" << endl;
        s << endl;
    }

    return str;
}
