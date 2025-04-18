-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local CultureName = require './common'

local male = {
	'Aiguo',
	'Bai',
	'Bingwen',
	'Bohai',
	'Bojing',
	'Bolin',
	'Boqin',
	'Changming',
	'Changpu',
	'Chao',
	'Chaoxiang',
	'Cheng',
	'Chenglei',
	'Chongan',
	'Chongkun',
	'Chonglin',
	'Chuanli',
	'Dai',
	'Delun',
	'Dingxiang',
	'Dong',
	'Donghai',
	'Duyi',
	'Enlai',
	'Fang',
	'Feng',
	'Fengge',
	'Fu',
	'Fuhua',
	'Gang',
	'Geming',
	'Gen',
	'Genghis',
	'Guang',
	'Guangli',
	'Gui',
	'Guiren',
	'Guoliang',
	'Guowei',
	'Hai',
	'Heng',
	'Hong',
	'Honghui',
	'Hongqi',
	'Huan',
	'Huojin',
	'Jian',
	'Jiang',
	'Jianguo',
	'Jianjun',
	'Jianyu',
	'Jingguo',
	'Jinhai',
	'Jinjing',
	'Junjie',
	'Kang',
	'Lei',
	'Liang',
	'Ling',
	'Liu',
	'Liwei',
	'Longwei',
	'Mengyao',
	'Mingli',
	'Minsheng',
	'Minzhe',
	'Nianzu',
	'Peng',
	'Pengfei',
	'Qianfan',
	'Qiang',
	'Qingshan',
	'Qingsheng',
	'Qiqiang',
	'Qiu',
	'Quan',
	'Renshu',
	'Rong',
	'Ru',
	'Shan',
	'Shanyuan',
	'Shen',
	'Shi',
	'Shining',
	'Shirong',
	'Shoushan',
	'Shunyuan',
	'Siyu',
	'Tao',
	'Tengfei',
	'Tingzhe',
	'Wei',
	'Weimin',
	'Weisheng',
	'Weiyuan',
	'Weizhe',
	'Wencheng',
	'Wenyan',
	'Wuzhou',
	'Xiang',
	'Xianliang',
	'Xiaobo',
	'Xiaodan',
	'Xiaojian',
	'Xiaosheng',
	'Xiaosi',
	'Xiaowen',
	'Xin',
	'Xing',
	'Xiu',
	'Xue',
	'Xueqin',
	'Xueyou',
	'Yang',
	'Yanlin',
	'Yaochuan',
	'Yaoting',
	'Yaozu',
	'Ye',
	'Yi',
	'Yingpei',
	'Yong',
	'Yongliang',
	'Yongnian',
	'Yongrui',
	'Yongzheng',
	'You',
	'Yuanjun',
	'Yunxu',
	'Yusheng',
	'Zedong',
	'Zemin',
	'Zengguang',
	'Zhen',
	'Zhengsheng',
	'Zhiqiang',
	'Zhong',
	'Zian',
	'Zihao',
	'Zongmeng'
}

local female = {
	'Baozhai',
	'Biyu',
	'Changchang',
	'Changying',
	'Chenguang',
	'Chunhua',
	'Chuntao',
	'Cuifen',
	'Daiyu',
	'Dandan',
	'Dongmei',
	'Ehuang',
	'Fang',
	'Fenfang',
	'Hong',
	'Hualing',
	'Huan',
	'Huian',
	'Huidai',
	'Huifang',
	'Huifen',
	'Huilang',
	'Huiliang',
	'Huiqing',
	'Huizhong',
	'Jia',
	'Jiao',
	'Jiayi',
	'Jiaying',
	'Jie',
	'Jing',
	'Jingfei',
	'Jinghua',
	'Juan',
	'Lan',
	'Lanfen',
	'Lanying',
	'Lifen',
	'Lihua',
	'Lijuan',
	'Liling',
	'Lin',
	'Ling',
	'Liqin',
	'Liqiu',
	'Liu',
	'Luli',
	'Mei',
	'Meifen',
	'Meifeng',
	'Meihui',
	'Meili',
	'Meilin',
	'Meirong',
	'Meixiang',
	'Meixiu',
	'Mingxia',
	'Mingyu',
	'Mingzhu',
	'Ning',
	'Ninghong',
	'Niu',
	'Nuo',
	'Nuying',
	'Peijing',
	'Peizhi',
	'Qi',
	'Qiang',
	'Qiao',
	'Qiaohui',
	'Qiaolian',
	'Qing',
	'Qingge',
	'Qingling',
	'Qingzhao',
	'Qiu',
	'Qiuyue',
	'Renxiang',
	'Rong',
	'Rou',
	'Ruiling',
	'Ruolan',
	'Ruomei',
	'Shan',
	'Shaoqing',
	'Shihong',
	'Shu',
	'Shuang',
	'Shuchun',
	'Shun',
	'Song',
	'Suyin',
	'Ting',
	'Weici',
	'Wen',
	'Wenling',
	'Wenqian',
	'Xia',
	'Xiang',
	'Xiaodan',
	'Xiaofan',
	'Xiaohui',
	'Xiaojian',
	'Xiaojing',
	'Xiaoli',
	'Xiaolian',
	'Xiaoling',
	'Xiaoqing',
	'Xiaosheng',
	'Xiaotong',
	'Xiaowen',
	'Xiaozhi',
	'Xifeng',
	'Xingjuan',
	'Xiu',
	'Xiulan',
	'Xiurong',
	'Xiuying',
	'Xue',
	'Xueman',
	'Ya',
	'Yan',
	'Yanlin',
	'Yanmei',
	'Yanyu',
	'Ying',
	'Yingtai',
	'Yu',
	'Yuan',
	'Yubi',
	'Yue',
	'Yuming',
	'Yun',
	'Yunru',
	'Yusheng',
	'Zhaohui',
	'Zhenzhen',
	'Zhilan',
	'Zhu',
	'Zongying'
}

local surname = {
	'Ang',
	'Au-Yong',
	'Bui',
	'Cao',
	'Chai',
	'Chan',
	'Chang',
	'Chao',
	'Chen',
	'Cheng',
	'Cheung',
	'Chew',
	'Chieu',
	'Chin',
	'Chong',
	'Chou',
	'Cong',
	'Cuan',
	'Cui',
	'Deng',
	'Dong',
	'Duan',
	'Feng',
	'Foong',
	'Fung',
	'Gai',
	'Gan',
	'Gao',
	'Gauk',
	'Geng',
	'Gim',
	'Gok',
	'Gong',
	'Gou',
	'Guan',
	'Guang',
	'Gui',
	'Hao',
	'Hong',
	'Hou',
	'Hsiao',
	'Hua',
	'Huan',
	'Huang',
	'Huo',
	'Jiang',
	'Jiao',
	'Jing',
	'Jiu',
	'Juan',
	'Jue',
	'Kang',
	'Kau',
	'Khoo',
	'Kong',
	'Koo',
	'Kuai',
	'Kuang',
	'Kui',
	'Kwan',
	'Kwei',
	'Kwong',
	'Lam',
	'Lang',
	'Lao',
	'Lei',
	'Liang',
	'Liao',
	'Lim',
	'Ling',
	'Liu',
	'Lui',
	'Mao',
	'Meng',
	'Miao',
	'Min',
	'Ming',
	'Moy',
	'Nao',
	'Nie',
	'Niu',
	'Ow-Yang',
	'Pang',
	'Pei',
	'Peng',
	'Pian',
	'Qian',
	'Qiao',
	'Qin',
	'Qing',
	'Qiu',
	'Quan',
	'Que',
	'Ruan',
	'Rui',
	'Sam',
	'Shao',
	'She',
	'Shen',
	'Sheng',
	'Shuai',
	'Shui',
	'Shum',
	'Shuo',
	'Su-Tu',
	'Sui',
	'Tai',
	'Tan',
	'Tang',
	'Thean',
	'Thien',
	'Tian',
	'Tong',
	'Tsang',
	'Tse',
	'Wang',
	'Wei',
	'Wen',
	'Weng',
	'Wong',
	'Woo',
	'Xian',
	'Xiao',
	'Xin',
	'Xuan',
	'Yang',
	'Yau',
	'Yin',
	'Yong',
	'You',
	'Yuan',
	'Yue',
	'Zeng',
	'Zhai',
	'Zhang',
	'Zhao',
	'Zhen',
	'Zhong',
	'Zhou'
}

local Chinese = CultureName.New({
	male = male,
	female = female,
	surname = surname,
	name = "Chinese",
	code = "zh",
})

return Chinese
