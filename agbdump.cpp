#include <iomanip>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>

#include <sndfile.h>

#include "agbplay/src/ConfigManager.h"
#include "agbplay/src/PlayerContext.h"
#include "agbplay/src/Rom.h"
#include "agbplay/src/SoundData.h"

int main(int argc, char *argv[])
{
    Rom::CreateInstance(argv[1]);
    ConfigManager::Instance().Load();
    ConfigManager::Instance().SetGameCode(Rom::Instance().GetROMCode());
    std::cout << "ROM code: " << Rom::Instance().GetROMCode() << std::endl;
    SongTable songTable;
    std::cout << "Number of songs: " << songTable.GetNumSongs() << std::endl;

    for (size_t i = 0; i < songTable.GetNumSongs(); ++i) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << i << ".wav";
        std::string filename = ss.str();

        std::cout << filename << std::endl;

        PlayerContext ctx(1, 16, EnginePars(15, 0, 4));
        ctx.InitSong(songTable.GetPosOfSong(i));

        size_t blocksRendered = 0;
        size_t nBlocks = ctx.mixer.GetSamplesPerBuffer();
        size_t nTracks = ctx.seq.tracks.size();
        std::vector<std::vector<sample>> trackAudio;

        SF_INFO oinfo;
        memset(&oinfo, 0, sizeof(oinfo));
        oinfo.samplerate = STREAM_SAMPLERATE;
        oinfo.channels = 2;    // sterep
        oinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        SNDFILE *ofile = sf_open(filename.c_str(), SFM_WRITE, &oinfo);
        if (ofile == NULL) {
            return 1;
        }
        std::vector<sample> renderedData(nBlocks);

        while (true) {
            ctx.reader.Process();
            ctx.mixer.Process(trackAudio);
            if (ctx.HasEnded())
                break;
            // mix streams to one master
            assert(trackAudio.size() == nTracks);
            // clear mixing buffer
            fill(renderedData.begin(), renderedData.end(), sample{0.0f, 0.0f});
            // mix all tracks to buffer
            for (std::vector<sample> &b : trackAudio) {
                assert(b.size() == renderedData.size());
                for (size_t i = 0; i < b.size(); i++) {
                    renderedData[i].left += b[i].left;
                    renderedData[i].right += b[i].right;
                }
            }
            sf_count_t processed = 0;
            do {
                processed += sf_writef_float(
                        ofile, &renderedData[processed].left,
                        sf_count_t(nBlocks) - processed);
            } while (processed < sf_count_t(nBlocks));
            blocksRendered += nBlocks;
        }
    }
}
