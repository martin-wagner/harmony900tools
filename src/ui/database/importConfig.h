// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QList>
#include <QString>

namespace database {

enum class StepState {
    Idle,
    Running,
    Done,
    Failed,
    Skipped
};

struct SourceSpec {
    QString fileName;
    QString location;   ///< url or local path
    bool isLocal = false;
};

/**
 * Everything the controller needs to start an import.
 * The order of sources matches the rows in the ui: the data files first,
 * the checksum file is stored separately.
 */
struct ImportConfig {
    QString targetDir;
    QList<SourceSpec> sources;
    bool verifyChecksums = true;
    SourceSpec checksums;
};

}
