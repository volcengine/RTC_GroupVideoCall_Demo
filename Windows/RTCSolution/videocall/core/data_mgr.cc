#include "data_mgr.h"
namespace videocall {

DataMgr& DataMgr::instance() {
    static DataMgr mgr;
    return mgr;
}

// {zh} 场景数据初始化
// {en} Scene data initialization
void DataMgr::init() {
    instance().mute_video_ = false;
    instance().mute_audio_ = false;
    instance().share_quality_index_ = 0;
}

}  // namespace videocall
