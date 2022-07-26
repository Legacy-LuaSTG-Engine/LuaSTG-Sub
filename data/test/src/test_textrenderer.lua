local test = require("test")

---@param ttfname string
---@param text string
---@param x number
---@param y number
---@param rot number
---@param hscale number
---@param vscale number
---@param blend lstg.BlendMode
---@param color lstg.Color
---@return number, number
function RenderTTF3(ttfname, text, x, y, rot, hscale, vscale, blend, color, ...)
    -- 警告：这里的实现代码以后可能会变化，甚至转为 C++ 实现
    -- 警告：请勿直接使用这些 Native API
    
    -- 翻译对齐
    local args = {...}
    local halign = 0
    local valign = 0
    for _, v in ipairs(args) do
        if v == "center" then
            halign = 1
        elseif v == "right" then
            halign = 2
        elseif v == "vcenter" then
            valign = 1
        elseif v == "bottom" then
            valign = 2
        end
    end

    -- 设置字体
    
    local fr = lstg.FontRenderer
    fr.SetFontProvider(ttfname)
    fr.SetScale(hscale, vscale)

    -- 计算笔触位置

    local x0, y0 = x, y
    local l, r, b, t = fr.MeasureTextBoundary(text)
    local w, h = r - l, t - b
    if halign == 0 then
        x = x - l -- 使左边缘对齐 x
    elseif halign == 1 then
        x = (x - l) - (w / 2) -- 居中
    else -- "right"
        x = x - r -- 使右边缘对齐 x
    end
    if valign == 0 then
        y = y - t -- 使顶边缘对齐 y
    elseif valign == 1 then
        y = (y - b) - (h / 2) -- 居中
    else -- "bottom"
        y = y - b -- 使底边缘对齐 y
    end

    -- 对笔触位置进行旋转

    local cos_v = math.cos(math.rad(rot))
    local sin_v = math.sin(math.rad(rot))
    local dx = x - x0
    local dy = y - y0
    local x1 = x0 + dx * cos_v - dy * sin_v
    local y1 = y0 + dx * sin_v + dy * cos_v

    -- 绘制

    local r, x2, y2 = fr.RenderTextInSpace(
        text,
        x1, y1, 0.5,
        math.cos(math.rad(rot))     , math.sin(math.rad(rot))     , 0,
        math.cos(math.rad(rot - 90)), math.sin(math.rad(rot - 90)), 0,
        blend, color
    )
    assert(r)

    return x2, y2
end

---@class test.Module.TextRenderer : test.Base
local M = {}

function M:onCreate()
    local old_pool = lstg.GetResourceStatus()
    lstg.SetResourceStatus("global")
    lstg.LoadTTF("Sans", "C:/Windows/Fonts/msyh.ttc", 32, 32)
    lstg.SetResourceStatus(old_pool)
end

function M:onDestroy()
    lstg.RemoveResource("global", 8, "Sans")
end

function M:onUpdate()
end

local long_text = [[羊茅蛹筛蓉
守荸楞檩攻
黍粟嚎向敬
构票罪灰柳
巨蚣迄痒时
现全华尘近
丙杆抑托忍
橡阵铐成歪
耐扑富吓禁
陵已徘联滓
掷囤克唇凸
那沾溺荐汞
磅窒扮腊衔
恃刽沃浪盛
律崭瘸限求
山荠可烘茵
源键贴下剪
拂丽勉四模
里九诀夸疮
叁喘倡厨告
揭祝特治微
高查擒搓澎
哈贰佛晨阎
剿墙捻抽哭
帐瞪擦凫赖
秫飞涩赴码
啃龙咱每汪
西缭片叙钾
庐哨场樱仰
琴抬幸杭鼓
廉鹰沟之郊
辨掌期慧十
法垫珊翻桌
舟宰裕述吃
路淑哄凉奖
恭沙争酗逛
荷幽陷俐恍
匹狠啡涤二
慌淌菩梁邪
馍站要骆酥
让使确防笤
膳遥唾愚泰
扬辱化唱臂
坝琉迁享既
贺慷昂赦蔼
妓混爪州保
笋俭然救埠
崖视促脐古
暂铡瞭渗这
刊氧侥挣度
紧降陡悔汇
汹嫩姥齐拗
银响吉甘孩
循露玩坚瘫
氏仲判喊贞
数堵奋驼转
烁正体堕蟋
骚利败妹睛
际根筹亚谓
硕索邮馁当
妒拯抚脂晒
造贮呕爵褥
详直暑请竭
方燎蔓嘲昨
胚仇皆蜜短
恤膛瀑瞳王
原菌待课幔
鲜块斯咨访
需糜者蝗浅
骡坑犀磁屑
萄刷旭嚣袜
鹉蹈昼售娱
嘶荤众擅嘱
劣扣兴泳梅
钳驶党轻鞠
礁涮综霍略
滥壁型侠藻
沛埂楣丹橘
脾橄巡菜杉
割矗钧帚谋
谁粗支淹付
熊脚墓稀筒
甩旋梢级农
辣囱屈合销
锻冶撮蔚颈
易甚遵镐牲
必墨策乔午
鸣切赚糙脓
业尿铲拦规
莹惨奠贝驳
踱捍衫零拥
国腥撇议莫
毫障毛杠臀
捆锰罐荚诊
道灸巷瓦充
艾肛扫宣磨
森本贱僻板
倔奶运姑岖
柿过诗壤夭
傲歉削巫趟
兰鹏早峡溜
疙逢哎师胎
倒乎吭阶姊
蒜捡耻誊再
醒点聘脏妖
峦垒缕有斥
稿侨史炼绵
逆了军披桑
殿孵估费曲
秸钙消恰滨
柒薛锣俩岛
间矫踩惕溢
关假碑宗吠
在汗逗厕卓
摔肿渔其牢
讶萨务轧督
颂荆巾谭领
批岭棠牙灶
歹己凄船袒
盈豁晤适碟
玉款裆概学
匕拾渊白局
胡缸弃它昵
蕉槐并囚朗
个普纵摧诡
猖想汉雨们
欢幕瞎虑量
亮炎殴振停
陶钮肋析镜
胆秩段赠遗
丁表婴专沦
衬热某苇惧
挤孔拔湖牵
跪浦挠劝轴
饱遮魂攀尔
胜萤献耿敏
页占涝芦箫
膨放旗离哀
饵笛像仪禽
袱库稻纯癌
峻艰垦膊韩
屠戏旱捂桅
桶密蕊红阻
伴卢设洁卵
臣长碴卑浑
铣胞踢大蜒
澜捎拒靡小
蟀栖蓝框肄
蛾骑植娩蔬
额砚卖五蛋
雌彬财逼熔
刻玷役嫌鲁
帜太镣缰瓤
房呆沫锐泛
摩翔教顽缺
散圆注睹屡
嘉怨胰派钝
绽珠顾进审
兔监捧症剧
著绘币铝掠
栋滔犁酌阔
康岸泵汤缴
次移魔宅往
薇拭忌依横
霉凝宁籽撑
伏晕矛妆物
犬埋候绍刃
屋宜图族拿
帕牺煌邢寓
阅儡频愧徐
诈愕如百迹
搂买菱糖介
翰弦免肩肪
悄鞭机玄奥
牡养抡努押
缅婆距解伍
娶申茧苍召
芯身狞陆弹
准奈穗洼畦
摊狼意咧委
蕴癞爆把队
腹菊晚庭嗓
苗须谐休住
弊庄宴昆唆
庆鬼输信坡
览萝隙辉鞋
严隅矿厢仅
共谆实姜祠
盖蒂箕秀糠
跌稳瞻淘煮
税卜恋镀栓
芍辽呐括赃
援缆捐驮知
测辙歼邓单
旨典连闺苦
滩钓床钻穴
沮锚供豫任
式翁盐阴调
丈闭茫绑池
捷才完柱透
园慢贬斤系
囊装困畔榛
焊妻轨笙迅
陈落寻恶腔
凛窥摆筝炬
挽赶愤欠榄
惹居浴纫躲
儒另舌词重
哪刺峰冤颅
忠伯炉爸趴
袍预戈嘁胸
神蓄欧练文
思遏未禀亥
蝇沥晴夯蠕
媚由杨聊组
酸辅篡羞葬
噩奔树熟坯
疯默耽咖豹
架跑氢椭笔
推讳颜胀畏
逞爱纷明江
坦蝶饮龟宏
吏射茂逾酒
齿润捺友拘
捉跺炭识骄
芒家丐弯尽
升乡蒲涯鸟
毙垮艺垢淡
蔗诚涛民梭
鹅啤鸡溉猾
曹塘采鳄程
淤春柏剃归
欺别绅昙啸
项唠梨奸梆
劈碰茸埃矩
股虾值涂懦
拱及堰岂甸
唬扯聪焰尸
碧喻持酱懒
枕越幼麸丝
洒背替辫阀
麻器勋簸虹
擎褐湿崔樟
俘撵或况绝
你誓粮蹭耗
懊警平菇瑞
坎暗珍枫删
伊翼衡咸肥
唯箩坐于毒
寸瓶扎门辰
睬剖甫翩坛
媒锤袁痪始
肢赛凿廷烫
雏掺千戚猛
痕蛆挎番敛
轰退勾鹿斑
雪疟励锌拣
与跳激乘鸭
扼丸沉棉跨
憾录省列泥
贾辟符旺事
碳葛培蜈举
立锯壳窖残
建宇凡扛累
号黄锡冕咒
穷赢真水隐
秽世记兵卒
仿逻蹂迫跃
独夫迎蚂光
阿苏孤痴逐
感愿对两走
署泽肝膘棱
傍罢隔挑驾
跷咙显终馅
啰雁猴蜂膜
围嚷啥盾屯
吁绩蝎掸磷
厅恒褂呜携
枢唁懈勺擂
俄夷盔药交
赌灼雄叛楷
曙病涣奉匪
鉴贵搅昏土
编裤冰螺首
寄置疾咪东
鸠毕僧冈蘑
哑徒便锁舅
结狐舷灌冷
抠蛛幅泼末
渠盅医刨烛
斋坪疹恬藕
复逸顶肯深
例秕塑攘辈
品猎掉赁习
封堂花棍陋
豆害酷滤赤
蚜娜儿钢草
芳芬革艳芋
贼日不照灭
橙宋澳海景
哮故非壹靖
从躬屉损钉
风脯厌姆椒
宽嗦避回线
蜗弄搞杂死
尾属吊征堡
毡女搁淋很
盒址耸枣途
脸鄙闷橱枯
天铃晦牧快
伸环焦衍漩
足崇外炮楔
秘蚊尊潮甥
盼元声此厚
端溯喳糟撒
搪味漂畸协
嗅幌肺赵滞
惜猬室蛀够
亏窿髓箭贡
窑盟烦簇材
街徽浆雇异
优茶屁毯怕
徙咕窗嫁研
屎稼含凰郭
伟先券糊卡
咽顷萍宝荣
茉嘿衙份纱
偷口渣雾沸
济贿苫轿飘
迟褒疑吱笑
粹帆备宿硝
境巧育晌魏
娄悬祟偶挚
字作冀包兆
粉府厘啦孕
朴旅愈箱将
粱各蚓惯遣
虽刀迂绿半
衣缨搜饶鳖
慕桨伐储弧
涌憋伦找吗
牛呈搀暇绣
萎帅谒稠炊
砍反觉而止
棵户卤狱骨
栏碌棘扳拧
的何帮唐广
涕谤胃年续
河仍腕许侯
驱姨观护梧
灿株斟增得
灵厉挖则拄
魁叮隶抄头
洲硅榔存鳍
缎技产敦悼
亭伙倦蘸恨
偏兑嘀践掰
画留竖性瞧
溅远几炕员
眷雕苔淫奏
么幢怔饿招
挥柔投固凑
侍倾丢虐逃
憎艇送状虏
负起蛮罗阱
讨眨渐恢笨
匠素呵婚计
枪帝穿氛姻
燃豺撕缤洛
破豪威基蔫
骤蝙捕俗熄
愁伶萌躺秧
麦开蹦孙僵
浓博遭苞债
褪霞篮瓮哗
躏稍我鹊算
芹沼青盲贤
仁冒钠新榕
炫俱弟戳竿
伪描叔能福
抗殖皇奢芭
董荧哩棺获
部只舍蔽航
玖尝鳞煞瓢
账乞瘟港吼
脊织搭绪翠
睁油蹋躁拖
校讯管违慈
哲竟琅锅宛
敷肉出愉命
酝腋乾靠恩
所除烙针钥
袭滚魄浙看
敲淀改豌茄
返挫殷清速
猪疗涡棒燕
佑爬卷惩吨
屿舞垂玲漠
掘淮倚焙达
趾评苟抱侵
应砌捏怒贫
婿禾荞怀滴
脖桐衅抖鲸
刮祖泄吆铅
氓亲匾虚谅
紊修划巩纳
秒人久庶认
面郁英颖棚
决铺般姿咐
蟆历莺拴瓷
沧叼急吻椰
脑涨错淆漆
秋庸钩匣沪
衷芜惫揩检
添澈拷辛约
咆黔掏奴覆
狰节力没摸
句遇地撰手
权偎污震贸
鬓檀通蚌换
啼击案纹拇
呻京寡万凌
善耀倘洞鲫
契整寒羔些
肮乱参悯匙
见垄泣荡格
握证祷悍悠
轮缀潘诽勤
斜竣动拙允
墅壕乍隘讼
折吮老城遍
弓泉枚档莱
岔绒操到雳
沿盹匿檬谱
碉念乳届雅
喜叽坠蹬熬
率曼剩雷贯
枝听餐蒋月
经趁束溪融
痰跟嗜闯莲
怯潭忙摇颊
呼枉写檐瞒
缔霜腿颤涉
馆报被嫡狂
耍昭醋傅宪
谚钦絮揉惶
椿措忱嫉勃
狸扩侣井仙
誉杜软焚蜻
慎篷签情挟
舶临铜弥色
公螟吟谴畅
憨祸舰楼筏
帽啄术否羡
扰盯颠鸥柄
癣驯科骂惑
刹村逮满套
鼎吵侧潦凯
笆乖榨烧彭
引驰寥璃琳
鸿播舔杯团
陌猩导毁薯
栈烹浇悟塌
超攒诺条若
服映象碍暴
抓还慨伤战
柑狗氮熏葡
瘤聚霎撞榴
拆咬带弛堪
砂担摹扶译
筋饭刚竞湃
货耙食韵刑
鹦主三抹娃
互腌容邻泌
墩剥烂匀北
卧蟹脉垃漾
凶惊玫陪柠
木似入臭堤
翅纽芽挡撬
绞尼佩庇桩
第晃范登疚
赫刁施嘹樊
榆谍悴载脱
酵腐蟥洋顺
迷睦暖锉疤
武吩父是粪
辖奕撼硫夜
茬极璧智锈
铆鸦弱察夏
益狈肖蚤缚
毅焕耘掐哟
更偿翎媳圾
瓜疏疫健锹
眠膏懂轩挨
蚀柬验诵藏
祈彻统缩旁
翘旦蔑诲怖
纪漏追说忧
就代母松遂
资簿虎询叫
勇精钞舀庞
双岩纸胶彩
姚壮延样扒
玛讲剔萧金
催尉瞄桦袄
随串茎沈车
一屹闲坏趋
周砸私孝茁
生汛来渤捞
布抢男舵赊
硼盆德疲辐
赏伞荒喝步
失昌兢孟骗
洽熙险积圣
恐楚闹鸽津
扔涵岳势歧
呀气蚪雹流
侄怎填肌罚
娇洗龄皱繁
踪佣态美乙
桥去息借电
游锭粒台前
做营简闽汁
浊价以稚绎
啊火叨揍理
亡初棕馒渺
且仔网控坊
拼椅纺趣俯
殊政邦霹窜
抵柜酬颁妇
童乌舒蛉狡
季心钟梦坞
碱仆驻辕袖
择亩畴序腰
扇嗡界书徊
摄肚犯拟塞
歌贷论爽憔
市谎谢责辞
诅炒荔敢贪
仓蜀亿剂称
羽痢倍窄席
兜侦群忽葫
昔用厂波诬
瘩腾饼杈戒
六卸烟渡匆
劳示兄飒霸
抒浩均窟该
宠都纠箍目
顿尺盏瘪眯
甲自婶嫂仑
劫岁粘球晋
舆漫隧丑叹
馋叭辩巍糯
琢驴酿伺喂
释枷蝴拐绷
芥子瘾纤后
蚯川刘蛇赋
料田问杖链
患衰嗽触裁
角形谈汰蒙
盘内胯佃位
裂训郎篱蛤
祥辆黑呛缠
胖陨眶坤标
滑牌栅净订
斩罕谬胧赡
械崎彼泊菠
锦彪肘谦坟
继皮版讽皂
卿祭铁犹叠
蒸噪截廓却
幻泻湾士唉
减邀酣沐苛
恕氨朱潜冯
林痹好戴眼
鱼聋粤星良
鸵鹃勿吸揪
余玻凭痘夺
揖究即聂挺
锋闸袋行纲
秤爹讹总秃
裹朦拨葱企
肴瞬较搏考
妈烤边赎喉
印壶蝌郑瑰
泪变晾糕晶
废葵仗南陕
赐呢嘴踊篇
勒断邑欣寨
阐鼻温秦臼
氯吝窘薄栗
香旷义迈芝
朝尤琼受耕
读语突逝薪
吕掀窍晓他
铛盗矮疼肤
吴乒哺杀液
朽干剑怪碘
矢凤茴淳鹤
铸涎拉肠指
]]
local long_text_lines_1 = {}
local long_text_lines_0 = {}
local long_text_lines = {}
local new_long_text = {}
for s in string.gmatch(long_text, "[^\n]+") do
    table.insert(long_text_lines_1, s)
    table.insert(long_text_lines_0, math.random(#long_text_lines_0),s)
    table.insert(long_text_lines, s)
    if #long_text_lines >= 16 then
        table.insert(new_long_text, table.concat(long_text_lines, " "))
        for i = 1, 16 do
            table.remove(long_text_lines)
        end
    end
end
local long_text_1 = table.concat(long_text_lines_1, "")
local long_text_0 = table.concat(long_text_lines_0, "")
local long_text = table.concat(new_long_text, "\n")

local x = false

function M:onRender()
    --set_camera()

    --local k = lstg.Input.Keyboard
    --local m = lstg.Input.Mouse
    
    --local kk1 = k.GetKeyState(k.X)
    --local mk1, mk2, mk3, mk4, mk5 = m.GetKeyState(m.Left), m.GetKeyState(m.Middle), m.GetKeyState(m.Right), m.GetKeyState(m.X1), m.GetKeyState(m.X2)
    --local mx1, my1 = lstg.Input.Mouse.GetPosition()
    --local mx2, my2 = lstg.Input.Mouse.GetPosition(true)
    --local mz = lstg.Input.Mouse.GetWheelDelta()
    --
    --local st = string.format("%s\n%s, %s, %s, %s, %s\n%.2f, %.2f\n%.2f, %.2f\n%.2f",
    --    kk1,
    --    mk1, mk2, mk3, mk4, mk5,
    --    mx1, my1,
    --    mx2, my2,
    --    mz
    --)

    local c_white = lstg.Color(255, 255, 255, 255)

    --lstg.RenderTTF("Sans", st, 0, 0, 720, 720, 0 + 0, c_white, 2)

    --local va, vb, vc = lstg.GetVersionNumber()

    --lstg.RenderTTF("Sans", string.format("版本号 %d.%d.%d", va, vb, vc), 0, 0, 64, 64, 0 + 8, c_white, 2)
    --lstg.RenderTTF("Sans", lstg.GetVersionName(), 0, 0, 0, 0, 0 + 8, c_white, 2)
    --lstg.RenderTTF("Sans", string.format("timer: %d", timer), 0, 0, 32, 32, 0 + 8, c_white, 2)

    --lstg.RenderTTF("Sans", "您好，别来无恙啊！", 0, 0, 720, 720, 0 + 0, lstg.Color(255, 0, 0, 0), 2)

    --RenderTTF3(
    --    "Sans", "旋转测试 ++++ 测试文本，。……",
    --    window.width / 2, window.height / 2,
    --    timer,
    --    4, 4,
    --    "", lstg.Color(255, 0, 0, 0),
    --    "center", "vcenter")

    window:applyCameraV()

    local str = long_text
    if not x then
        x = true
        str = long_text_0
    end
    --lstg.RenderTTF("Sans", str, 0, window.width, 0, window.height, 0 + 0 + 16, c_white, 2)

    lstg.FontRenderer.SetFontProvider("Sans")
    lstg.FontRenderer.SetScale(0.5, 0.5)
    lstg.FontRenderer.RenderText(str, 0, window.height - lstg.FontRenderer.GetFontAscender(), 0.5, "", c_white)
end

test.registerTest("test.Module.TextRenderer", M)
