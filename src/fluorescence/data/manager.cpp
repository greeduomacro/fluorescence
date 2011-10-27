
#include "manager.hpp"

#include <boost/filesystem/operations.hpp>

#include "tiledataloader.hpp"
#include "artloader.hpp"
#include "huesloader.hpp"
#include "gumpartloader.hpp"
#include "maploader.hpp"
#include "staticsloader.hpp"
#include "maptexloader.hpp"
#include "animdataloader.hpp"
#include "animloader.hpp"
#include "mobtypesloader.hpp"
#include "unifontloader.hpp"
#include "deffileloader.hpp"

#include <ui/singletextureprovider.hpp>
#include <ui/animdatatextureprovider.hpp>

#include <typedefs.hpp>

namespace fluo {
namespace data {

Manager* Manager::singleton_= NULL;

bool Manager::create(Config& config) {
    if (!singleton_) {
        try {
            singleton_ = new Manager();
            singleton_->init(config);
        } catch (const std::exception& ex) {
            LOG_EMERGENCY << "Error initializing data::Manager: " << ex.what() << std::endl;
            return false;
        }
    }

    return true;
}

void Manager::destroy() {
    if (singleton_) {
        delete singleton_;
        singleton_ = NULL;
    }
}

Manager* Manager::getSingleton() {
    if (!singleton_) {
        throw Exception("fluo::data::Manager Singleton not created yet");
    }

    return singleton_;
}

Manager::Manager() {
}

void Manager::init(Config& config) {
    buildFilePathMap(config);

    std::map<std::string, boost::filesystem::path>::iterator iter = filePathMap_.begin();
    std::map<std::string, boost::filesystem::path>::iterator end = filePathMap_.end();

    for (; iter != end; ++iter) {
        LOG_DEBUG << "Path for \"" << iter->first << "\" is " << iter->second << std::endl;
    }


    boost::filesystem::path idxPath;
    boost::filesystem::path path;
    boost::filesystem::path difOffsetsPath;
    boost::filesystem::path difIdxPath;
    boost::filesystem::path difPath;

    checkFileExists("tiledata.mul");
    path = filePathMap_["tiledata.mul"];
    LOG_INFO << "Opening tiledata.mul from mul=" << path << std::endl;
    tileDataLoader_.reset(new TileDataLoader(path));

    checkFileExists("hues.mul");
    path = filePathMap_["hues.mul"];
    LOG_INFO << "Opening hues.mul from mul=" << path << std::endl;
    huesLoader_.reset(new HuesLoader(path));

    checkFileExists("texidx.mul");
    checkFileExists("texmaps.mul");
    idxPath = filePathMap_["texidx.mul"];
    path = filePathMap_["texmaps.mul"];
    LOG_INFO << "Opening maptex from idx=" << idxPath << " mul=" << path << std::endl;
    mapTexLoader_.reset(new MapTexLoader(idxPath, path));

    checkFileExists("artidx.mul");
    checkFileExists("art.mul");
    idxPath = filePathMap_["artidx.mul"];
    path = filePathMap_["art.mul"];
    LOG_INFO << "Opening art from idx=" << idxPath << " mul=" << path << std::endl;
    artLoader_.reset(new ArtLoader(idxPath, path));

    checkFileExists("gumpidx.mul");
    checkFileExists("gumpart.mul");
    idxPath = filePathMap_["gumpidx.mul"];
    path = filePathMap_["gumpart.mul"];
    LOG_INFO << "Opening gump art from idx=" << idxPath << " mul=" << path << std::endl;
    gumpArtLoader_.reset(new GumpArtLoader(idxPath, path));

    checkFileExists("animdata.mul");
    path = filePathMap_["animdata.mul"];
    LOG_INFO << "Opening animdata from mul=" << path << std::endl;
    animDataLoader_.reset(new AnimDataLoader(path));


    unsigned int blockCountX;
    unsigned int blockCountY;
    bool difsEnabled;

    std::stringstream ss;
    for (unsigned int index = 0; index <= 5; ++index) {
        ss.str(""); ss.clear();
        ss << "/fluo/files/map" << index << "@enabled";

        if (!config[ss.str().c_str()].asBool()) {
            continue;
        }

        ss.str(""); ss.clear();
        ss << "/fluo/files/map" << index << "@width";
        LOG_DEBUG << "key: " << ss.str().c_str() << std::endl;
        blockCountX = config[ss.str().c_str()].asInt();

        ss.str(""); ss.clear();
        ss << "/fluo/files/map" << index << "@height";
        blockCountY = config[ss.str().c_str()].asInt();

        ss.str(""); ss.clear();
        ss << "/fluo/files/map" << index << "@difs-enabled";
        difsEnabled = config[ss.str().c_str()].asBool();

        ss.str(""); ss.clear();
        ss << "map" << index << ".mul";
        checkFileExists(ss.str());
        path = filePathMap_[ss.str()];
        if (difsEnabled) {
            ss.str(""); ss.clear();
            ss << "mapdifl" << index << ".mul";
            checkFileExists(ss.str());
            difOffsetsPath = filePathMap_[ss.str()];

            ss.str(""); ss.clear();
            ss << "mapdif" << index << ".mul";
            checkFileExists(ss.str());
            difPath = filePathMap_[ss.str()];

            LOG_INFO << "Opening map" << index << " from mul=" << path << ", dif-offsets=" << difOffsetsPath <<
                    ", dif=" << difPath << ", blockCountX=" << blockCountX << ", blockCountY=" << blockCountY << std::endl;
            mapLoader_[index].reset(new MapLoader(path, difOffsetsPath, difPath, blockCountX, blockCountY));
        } else {
            LOG_INFO << "Opening map" << index << " from mul=" << path << ", difs disabled, blockCountX=" <<
                    blockCountX << ", blockCountY=" << blockCountY << std::endl;
            mapLoader_[index].reset(new MapLoader(path, blockCountX, blockCountY));
        }

        if (!fallbackMapLoader_.get()) {
            fallbackMapLoader_ = mapLoader_[index];
        }


        ss.str(""); ss.clear();
        ss << "staidx" << index << ".mul";
        checkFileExists(ss.str());
        idxPath = filePathMap_[ss.str()];

        ss.str(""); ss.clear();
        ss << "statics" << index << ".mul";
        checkFileExists(ss.str());
        path = filePathMap_[ss.str()];

        if (difsEnabled) {
            ss.str(""); ss.clear();
            ss << "stadifl" << index << ".mul";
            checkFileExists(ss.str());
            difOffsetsPath = filePathMap_[ss.str()];

            ss.str(""); ss.clear();
            ss << "stadifi" << index << ".mul";
            checkFileExists(ss.str());
            difIdxPath = filePathMap_[ss.str()];

            ss.str(""); ss.clear();
            ss << "stadif" << index << ".mul";
            checkFileExists(ss.str());
            difPath = filePathMap_[ss.str()];

            LOG_INFO << "Opening statics" << index << " from idx=" << idxPath << ", mul=" << path << ", dif-offsets=" << difOffsetsPath <<
                    ", dif-idx=" << difIdxPath << ", dif=" << difPath << std::endl;
            staticsLoader_[index].reset(new StaticsLoader(idxPath, path, difOffsetsPath, difIdxPath, difPath, blockCountX, blockCountY));
        } else {
            LOG_INFO << "Opening statics" << index << " from idx=" << idxPath << ", mul=" << path << ", difs disabled" << std::endl;
            staticsLoader_[index].reset(new StaticsLoader(idxPath, path, blockCountX, blockCountY));
        }

        if (!fallbackStaticsLoader_.get()) {
            fallbackStaticsLoader_ = staticsLoader_[index];
        }
    }



    unsigned int highDetailCount;
    unsigned int lowDetailCount;
    const char* animNames[] = { "anim", "anim2", "anim3", "anim4", "anim5" };
    for (unsigned int index = 0; index < 5; ++index) {
        ss.str(""); ss.clear();
        ss << "/fluo/files/" << animNames[index] << "@enabled";

        if (!config[ss.str().c_str()].asBool()) {
            continue;
        }

        ss.str(""); ss.clear();
        ss << "/fluo/files/" << animNames[index] << "@highdetail";
        highDetailCount = config[ss.str().c_str()].asInt();

        ss.str(""); ss.clear();
        ss << "/fluo/files/" << animNames[index] << "@lowdetail";
        lowDetailCount = config[ss.str().c_str()].asInt();

        ss.str(""); ss.clear();
        ss << animNames[index] << ".idx";
        checkFileExists(ss.str());
        idxPath = filePathMap_[ss.str()];

        ss.str(""); ss.clear();
        ss << animNames[index] << ".mul";
        checkFileExists(ss.str());
        path = filePathMap_[ss.str()];

        LOG_INFO << "Opening " << animNames[index] << " from idx=" << idxPath << ", mul=" << path << ", high-detail=" <<
                highDetailCount << ", low-detail=" << lowDetailCount << std::endl;
        animLoader_[index].reset(new AnimLoader(idxPath, path, highDetailCount, lowDetailCount));

        if (!fallbackAnimLoader_.get()) {
            fallbackAnimLoader_ = animLoader_[index];
        }
    }

    checkFileExists("mobtypes.txt");
    path = filePathMap_["mobtypes.txt"];
    LOG_INFO << "Opening mobtypes.txt from path=" << path << std::endl;
    mobTypesLoader_.reset(new MobTypesLoader(path));


    const char* unifontNames[] = { "unifont", "unifont1", "unifont2", "unifont3", "unifont4", "unifont4", "unifont5", "unifont6", "unifont7", "unifont8", "unifont9", "unifont10", "unifont11", "unifont12" };

    for (unsigned int index = 0; index <= 12; ++index) {
        ss.str(""); ss.clear();
        ss << unifontNames[index] << ".mul";
        if (hasPathFor(ss.str())) {
            path = filePathMap_[ss.str()];
            LOG_INFO << "Opening " << unifontNames[index] << ".mul from path=" << path << std::endl;
            uniFontLoader_[index].reset(new UniFontLoader(path));
            if (!fallbackUniFontLoader_) {
                fallbackUniFontLoader_ = uniFontLoader_[index];
            }
        } else {
            LOG_WARN << "Unable to find " << unifontNames[index] << ".mul" << std::endl;
        }
    }


    checkFileExists("body.def");
    path = filePathMap_["body.def"];
    LOG_INFO << "Opening body.def from path=" << path << std::endl;
    bodyDefLoader_.reset(new DefFileLoader<BodyDef>(path));

    checkFileExists("bodyconv.def");
    path = filePathMap_["bodyconv.def"];
    LOG_INFO << "Opening bodyconv.def from path=" << path << std::endl;
    bodyConvDefLoader_.reset(new DefFileLoader<BodyConvDef>(path));
}

Manager::~Manager() {
    LOG_INFO << "data::Manager shutdown" << std::endl;
}

boost::shared_ptr<MapLoader> Manager::getMapLoader(unsigned int index) {
    if (index > 4) {
        LOG_WARN << "Trying to access map loader for map index " << index << std::endl;
        index = 0;
    }

    Manager* sing = getSingleton();

    boost::shared_ptr<MapLoader> ret = sing->mapLoader_[index];
    if (ret) {
        return ret;
    } else {
        LOG_WARN << "Trying to access uninitialized map index " << index << std::endl;
        return sing->fallbackMapLoader_;
    }
}

boost::shared_ptr<StaticsLoader> Manager::getStaticsLoader(unsigned int index) {
    if (index > 4) {
        LOG_WARN << "Trying to access statics loader for map index " << index << std::endl;
        index = 0;
    }

    Manager* sing = getSingleton();

    boost::shared_ptr<StaticsLoader> ret = sing->staticsLoader_[index];
    if (ret) {
        return ret;
    } else {
        LOG_WARN << "Trying to access uninitialized statics index " << index << std::endl;
        return sing->fallbackStaticsLoader_;
    }
}

boost::shared_ptr<ui::TextureProvider> Manager::getItemTextureProvider(unsigned int artId) {
    boost::shared_ptr<ui::TextureProvider> ret;

    // check for animate flag in tiledata
    if (getTileDataLoader()->getStaticTileInfo(artId)->animation()) {
        // if exists, load complex textureprovider
        ret.reset(new ui::AnimDataTextureProvider(artId));
    } else {
        // if not, just load the simple one
        ret.reset(new ui::SingleTextureProvider(ui::SingleTextureProvider::FROM_ART_MUL, artId));
    }

    return ret;
}

std::vector<boost::shared_ptr<ui::Animation> > Manager::getAnim(unsigned int bodyId, unsigned int animId) {
    Manager* sing = getSingleton();

    BodyConvDef bodyConvEntry = sing->bodyConvDefLoader_->get(bodyId);
    boost::shared_ptr<AnimLoader> ldr;
    unsigned int animIdx;
    if (bodyConvEntry.bodyId_ != 0) {
        LOG_DEBUG << "redirecting anim! body=" << bodyId << " fileIdx=" << bodyConvEntry.getAnimFileIdx() << " idx in file=" << bodyConvEntry.getAnimIdxInFile() << std::endl;
        animIdx = bodyConvEntry.getAnimIdxInFile();
        ldr = getAnimLoader(bodyConvEntry.getAnimFileIdx());
    } else {
        ldr = getAnimLoader(0);
        animIdx = bodyId;
    }

    std::vector<boost::shared_ptr<ui::Animation> > ret;

    boost::shared_ptr<ui::Animation> tmpDown = ldr->getAnimation(animIdx, animId, 0);
    boost::shared_ptr<ui::Animation> tmpDownLeft = ldr->getAnimation(animIdx, animId, 1);
    boost::shared_ptr<ui::Animation> tmpLeft = ldr->getAnimation(animIdx, animId, 2);
    boost::shared_ptr<ui::Animation> tmpUpLeft = ldr->getAnimation(animIdx, animId, 3);
    boost::shared_ptr<ui::Animation> tmpUp = ldr->getAnimation(animIdx, animId, 4);

    // mirrored
    ret.push_back(tmpUpLeft);
    ret.push_back(tmpLeft);
    ret.push_back(tmpDownLeft);

    ret.push_back(tmpDown);
    ret.push_back(tmpDownLeft);
    ret.push_back(tmpLeft);
    ret.push_back(tmpUpLeft);
    ret.push_back(tmpUp);

    return ret;
}

boost::shared_ptr<AnimLoader> Manager::getAnimLoader(unsigned int index) {
    if (index > 5) {
        LOG_WARN << "Trying to access anim loader with index " << index << std::endl;
        index = 0;
    }

    Manager* sing = getSingleton();

    boost::shared_ptr<AnimLoader> ret = sing->animLoader_[index];
    if (ret) {
        return ret;
    } else {
        LOG_WARN << "Trying to access uninitialized anim loader with index " << index << std::endl;
        return sing->fallbackAnimLoader_;
    }
}

boost::shared_ptr<UniFontLoader> Manager::getUniFontLoader(unsigned int index) {
    if (index > 12) {
        LOG_WARN << "Trying to access unifont loader with index " << index << std::endl;
        index = 0;
    }

    Manager* sing = getSingleton();

    boost::shared_ptr<UniFontLoader> ret = sing->uniFontLoader_[index];
    if (ret) {
        return ret;
    } else {
        LOG_WARN << "Trying to access uninitialized unifont loader with index " << index << std::endl;
        return sing->fallbackUniFontLoader_;
    }
}

BodyDef Manager::getBodyDef(unsigned int baseBodyId) {
    Manager* sing = getSingleton();

    // a little strange, but bodyconv.def overwrites body.def. So if there is a bodyconv entry, return foo
    if (sing->bodyConvDefLoader_->hasValue(baseBodyId)) {
        return BodyDef();
    }

    return sing->bodyDefLoader_->get(baseBodyId);
}

void Manager::buildFilePathMap(Config& config) {
    boost::filesystem::path mulDirPath = config["/fluo/files/mul-directory@path"].asPath();
    if (!boost::filesystem::exists(mulDirPath) || !boost::filesystem::is_directory(mulDirPath)) {
        throw Exception("Invalid mul directory");
    }

    addToFilePathMap(mulDirPath);
    boost::filesystem::path shardDataPath("shards");
    shardDataPath = shardDataPath / config["/fluo/shard@name"].asPath() / "data";
    addToFilePathMap(shardDataPath);
}

void Manager::addToFilePathMap(const boost::filesystem::path& directory) {
    namespace bfs = boost::filesystem;

    if (!bfs::exists(directory) || !bfs::is_directory(directory)) {
        return;
    }

    bfs::directory_iterator nameIter(directory);
    bfs::directory_iterator nameEnd;

    for (; nameIter != nameEnd; ++nameIter) {
        if (bfs::is_directory(nameIter->status())) {
            // no subdirectories
            continue;
        }

        UnicodeString str = StringConverter::fromUtf8(nameIter->path().filename());
        str.toLower();

        filePathMap_[StringConverter::toUtf8String(str)] = nameIter->path();
    }
}

bool Manager::hasPathFor(const std::string& file) const {
    return filePathMap_.find(file) != filePathMap_.end();
}

void Manager::checkFileExists(const std::string& file) const {
    if (!hasPathFor(file)) {
        LOG_ERROR << "Required file " << file << " was not found" << std::endl;
        throw std::exception();
    }
}

}
}
