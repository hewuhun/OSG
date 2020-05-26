/**************************************************************************************************
* @file ExLabelNode.h
* @note 标牌类，实现标牌的绘制
* @author g00034
* @data 2016-8-31
**************************************************************************************************/
#ifndef FE_EXTERN_LABEL_NODE_H
#define FE_EXTERN_LABEL_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExternNode.h>
#include <FeUtils/RenderContext.h>
#include <osgText/Text>
#include <FeExtNode/ExLabelBindingNodeCallback.h>

using namespace FeUtil;
using namespace osgEarth;


namespace FeExtNode
{
	/**
	  * @class CExLabelItemOption
	  * @note 标牌中子项的相关参数
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelItemOption : public CExternNodeOption
	{
	public:
		/**  
		  * @note 标牌子项配置的构造函数
		*/
		CExLabelItemOption();

		/**  
		  * @note 标牌子项配置的析构函数
		*/
		~CExLabelItemOption();

		/**  
		  * @note 赋值构造函数
		*/
		CExLabelItemOption& operator=(const CExLabelItemOption& opt);

	public:
		/// 文本对齐方式
		enum E_TEXT_ALIGNMENT
		{
			ALIGN_LEFT = 0,  // 左对齐
			ALIGN_CENTER,    // 居中
			ALIGN_RIGHT      // 右对齐
		};

	public:
		/**  
		  * @note 获取字体名称
		*/
		std::string& FontName(){return m_strFontName;}

		/**  
		  * @note 获取文本
		*/
		std::string& Text() {return m_strText;}
		
		/**  
		  * @note 获取文本颜色
		*/
		osg::Vec4& TextColor() {return m_vecTextColor;}

		/**  
		  * @note 获取背景颜色
		*/
		osg::Vec4& BackgroundColor() {return m_vecBackgroundColor;}
		
		/**  
		  * @note 获取字体大小
		*/
		float& FontSize() {return m_fFontSize;}

		/**  
		  * @note 获取显示宽度
		*/
		float& Width() {return m_fWidth;}

		/**  
		  * @note 获取显示高度
		*/
		float& Height() {return m_fHeight;}

		/**  
		  * @note 获取背景图
		*/
		osg::ref_ptr<osg::Image>& BackgroundImage() { return m_rpBackgroundImage; }

		/**  
		  * @note 获取背景图路径
		*/
		std::string& BackgroundImagePath() { return m_strImagePath; }

		/**  
		  * @note 获取鼠标悬浮显示图
		*/
		osg::ref_ptr<osg::Image>& HoverImage() { return m_rpHoverImage; }

		/**  
		  * @note 获取文本对齐方式
		*/
		E_TEXT_ALIGNMENT& Alignment() {return m_textAlign;}


	protected:
		/// 显示宽度
		float            m_fWidth;

		/// 显示高度
		float            m_fHeight;

		/// 文本对齐方式
		E_TEXT_ALIGNMENT m_textAlign;

		/// 文本字符串
		std::string      m_strText;

		/// 字体名称
		std::string      m_strFontName;

		/// 字体大小
		float            m_fFontSize;

		/// 文本颜色
		osg::Vec4        m_vecTextColor;

		/// 背景色
		osg::Vec4        m_vecBackgroundColor;

		/// 背景图图片
		osg::ref_ptr<osg::Image>  m_rpBackgroundImage;

		/// 背景图图片路径
		std::string     m_strImagePath;

		/// 鼠标悬浮显示图片
		osg::ref_ptr<osg::Image>  m_rpHoverImage;
	};

	class CExLabelItemNode;
    typedef std::list<osg::ref_ptr<CExLabelItemNode> > CExLabelItemNodeList;

	/**
	  * @class CExLabelItemNode
	  * @note 标牌子项节点
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelItemNode : public CExternNode
	{
		friend class CExLabelNode;
	public:
		/**  
		  * @note 标牌子项节点的构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param opt [in] 标牌子项节点的配置属性集合
		*/
		CExLabelItemNode(FeUtil::CRenderContext* pRender, CExLabelItemOption* opt);

		/**  
		  * @note 标牌子项节点的析构函数
		*/
		~CExLabelItemNode(){}

	public:
		virtual void Accept(CExternNodeVisitor& nv);
		/**  
		  * @note 显示/隐藏本节点下的所有孩子节点
		*/
		void SetChildItemVisible(bool bShow);
		bool IsChildItemVisible() { return m_bShowChild; }

		/**  
		  * @note 添加孩子节点
		*/
		FeUtil::FEID AddChildItem(const std::string& strItemText);

		/**  
		  * @note 添加孩子节点
		*/
		bool AddChildItem(CExLabelItemNode* pItem);

		/**  
		  * @note 获取孩子节点
		*/
		CExLabelItemNode* GetChildItem(FeUtil::FEID nodeID);

		/**  
		  * @note 删除孩子节点
		*/
		bool RemoveChildItem(FeUtil::FEID nodeID);

		/**  
		  * @note 获取字体名称
		*/
		const std::string& GetFontName();
		void SetFontName(const std::string& fontName);

		/**  
		  * @note 获取文本
		*/
// 		std::string GetName();
// 		void SetName(const std::string& strText);
		std::string GetText();
		void SetText(const std::string& strText);
		
		/**  
		  * @note 获取文本颜色
		*/
		const osg::Vec4& GetTextColor();
		void SetTextColor(const osg::Vec4& vecColor);

		/**  
		  * @note 获取背景颜色
		*/
		const osg::Vec4& GetBackgroundColor();
		void SetBackgroundColor(const osg::Vec4& vecColor);

		/**  
		  * @note 获取背景图
		*/
		void SetBackgroundImage(const std::string& strPath);
		void SetBackgroundImage(osg::Image* pImage);
		std::string GetBackgroundImage();
		void RemoveBackgroundImage();

		/**  
		  * @note 获取鼠标悬浮显示图
		*/
		void SetHoverImage(osg::Image* pImage);
		osg::Image* GetHoverImage();
		
		/**  
		  * @note 获取字体大小
		*/
		float GetFontSize();
		void SetFontSize(const float& fSize);

		/**  
		  * @note 获取显示宽度
		*/
		float GetWidth();
		void SetWidth(const float& fWidth);

		/**  
		  * @note 获取显示高度
		*/
		float GetHeight();
		void SetHeight(const float& fHeight);

		/**  
		  * @note 获取文本对齐方式
		*/
		CExLabelItemOption::E_TEXT_ALIGNMENT GetTextAlign();
		void SetTextAlign(CExLabelItemOption::E_TEXT_ALIGNMENT align);

		/**  
		* @note 获取子项节点列表
		*/
		CExLabelItemNodeList GetChildList();

	protected:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLabelItemOption* GetOption();

	private:
		/**  
		  * @note 创建文字节点
		*/
		void CreateTextNode();

		/**  
		  * @note 创建四边形背景节点
		*/
		void CreateQuadBackgroundNode();
		
		/**  
		  * @note 更新位置
		*/
		void UpdatePosition();

		/**  
		  * @note 更新文本显示和视口大小
		*/
		void UpdateTextAndViewPort();

		/**  
		  * @note 更新背景四边形显示
		*/
		void UpdateBackgroundQuad();

		/**  
		  * @note 设置位置，左上角位置
		*/
		void SetPosition(const osg::Vec2& vecPosNorm);

		/**  
		  * @note 获取位置，左上角位置
		*/
		const osg::Vec2& GetPosition(){ return m_vecLT;}

		/**  
		  * @note 判断指定屏幕坐标是否在该节点显示区域内
		*/
		bool isPtIn(const osg::Vec2& vecPosScreen);

		/**  
		  * @note 检查本节点和所有孩子节点，获取指定屏幕坐标下的标牌子项
		  * @return 若存在，则返回对应子项，否则返回0
		*/
		CExLabelItemNode* GetItemByPos(const osg::Vec2& vecPosScreen);

		/**  
		  * @note 求当前节点包括其子节点显示的最大宽度和高度
		*/
		void GetDiaplayMaxSize(float& fMaxWidth, float& fMaxHeight);

		/**  
		  * @note 获取当前视口大小的接口，方便类计算中使用
		*/
		void GetCurrentViewPort(float& fWidth, float& fHeight);
		const float GetCurrentViewPortWidth();
		const float GetCurrentViewPortHeight();
		
	private:
		/// 文本对象
		osg::ref_ptr<osgText::Text>   m_text; 

		/// 字体对象
		osg::ref_ptr<osgText::Font>   m_font; 

		/// 背景四边形对象
		osg::ref_ptr<osg::Geometry>   m_rpGeomQuad;

		/// 四边形顶点数组，顺序为LT,RT,RB,LB
		osg::ref_ptr<osg::Vec3Array>  m_rpQuadVertexes; 

		/// 四边形背景色数组
		osg::ref_ptr<osg::Vec4Array>  m_rpVecBkColor;

		/// 图形绘制geode节点
		osg::ref_ptr<osg::Geode>      m_rpGeodeRoot;

		/// 背景贴图
		osg::ref_ptr<osg::Texture2D>  m_rpBkImageTexture;

		/// 左上角位置
		osg::Vec2                     m_vecLT;

		/// 当前视口宽高
		osg::Vec2                     m_curViewPort;

		/// 是否显示子项
		bool                          m_bShowChild;

		/// 子节点列表
		CExLabelItemNodeList          m_listChildNode;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;

	};

	/**
	* @class CExLabelNodeOption
	* @note 标牌节点的配置选项
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelNodeOption : public CExLabelItemOption
	{
	public:
		/**  
		  * @note 标牌节点配置的构造函数
		*/
		CExLabelNodeOption(){}

		/**  
		  * @note 标牌节点配置的析构函数
		*/
	    virtual ~CExLabelNodeOption(){}

	public:
		/**  
		  * @note 获取绑定节点经纬高
		*/
		osg::Vec3d& BindingLLH() {return m_vecBindingLLH;}
		
	protected:
		/// 绑定点的经纬高坐标
		osg::Vec3d   m_vecBindingLLH;
	};

	/**
	* @class CExLabelNode
	* @note 标牌节点
			一个标牌中含有一个标头节点，还可以添加若干标牌子项节点
			每一个子项节点又可以包含若干孩子节点作为二级目录，依此递归可形成树状结构图
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelNode : public CExternNode
	{
	public:
		/**  
		  * @note 标牌节点的构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param opt [in] 标牌节点的配置属性集合
		*/
		CExLabelNode(FeUtil::CRenderContext* pRender, CExLabelNodeOption* opt);

		/**  
		  * @note 标牌节点的析构函数
		*/
		~CExLabelNode();

		/**  
		  * @note 增加标牌子项，位于标头底部
		*/
		FeUtil::FEID AddLabelItem(const std::string& strItemText);

		/**  
		* @note 新增标牌子项节点
		*/
		bool AddLabelItem(CExLabelItemNode* pItem);

		/**  
		  * @note 获取标牌子项
		*/
		CExLabelItemNode* GetLabelItem(FeUtil::FEID nodeID);

		/**  
		  * @note 删除标牌子项
		*/
		bool RemoveLabelItem(FeUtil::FEID nodeID);

		/**  
		  * @note 设置是否启用绑定
		*/
		void SetBindingEnable(const bool bEnableBinding);

		/**  
		  * @note 获取是否启用绑定
		*/
		bool IsBindingEnable();

		/**  
		  * @note 设置绑定点
		*/
		bool SetBindingPointLLH(const osg::Vec3d& pt);

		/**  
		  * @note 获取绑定点
		*/
		const osg::Vec3d& GetBindingPointLLH();

		/**  
		  * @note 绑定节点
		*/
		bool BindingNode(osg::Node* pNode);
		bool RemoveBindingNode(osg::Node* pNode);

		/**  
		* @note 响应点击事件  
		* @param vecPosScreen [in] 屏幕坐标
		* @param bDoubleClick [in] ture为双击，false为单击
		* @return 成功处理返回ture，否则返回false
		*/
		bool ClickLabel(const osg::Vec2& vecPosScreen, bool bDoubleClick);

		/**  
		* @note 判断屏幕坐标是否在标头节点上
		*/
		bool IsPtInTitle(const osg::Vec2& vecPosScreen);

		/**  
		* @note 判断是否需要重新绘制
		*/
		bool IsNeedRedraw() { return m_bNeedRedraw; }

		/**  
		* @note 响应拖拽事件  
		* @param vecCurPos [in]  当前鼠标位置
		* @param vecLastPos [in] 上一次鼠标位置
		* @return 无
		*/
		void DragLabel(const osg::Vec2& vecCurPosScreen, const osg::Vec2& vecLastPosScreen);

		/**  
		* @note 显示/隐藏标牌子项
		*/
		void ShowLabelItem(bool bShow);

		/**  
		* @note 根据当前的绑定位置等参数进行重新绘制
		*/
		void Redraw();

		/**  
		* @note 获取标题
		*/
// 		std::string GetName();
// 		void SetName(const std::string& strTitle);
		std::string GetText();
		void SetText(const std::string& strTitle);

		/**  
		* @note 获取标题颜色
		*/
		const osg::Vec4& GetTextColor();
		void SetTextColor(const osg::Vec4& vecColor);

		/**  
		* @note 获取背景颜色
		*/
		const osg::Vec4& GetBackgroundColor();
		void SetBackgroundColor(const osg::Vec4& vecColor);

		/**  
		* @note 获取背景图
		*/
		void SetBackgroundImage(const std::string& strPath);
		void SetBackgroundImage(osg::Image* pImage);	
		std::string GetBackgroundImage();
		void RemoveBackgroundImage();

		/**  
		* @note 获取字体大小
		*/
		float GetFontSize();
		void SetFontSize(const float& fSize);

		/**  
		  * @note 获取字体名称
		*/
		const std::string& GetFontName();
		void SetFontName(const std::string& fontName);

		/**  
		* @note 获取显示宽度
		*/
		float GetWidth();
		void SetWidth(const float& fWidth);

		/**  
		* @note 获取显示高度
		*/
		float GetHeight();
		void SetHeight(const float& fHeight);

		/**  
		* @note 获取标题对齐方式
		*/
		CExLabelItemOption::E_TEXT_ALIGNMENT GetTextAlign();
		void SetTextAlign(CExLabelItemOption::E_TEXT_ALIGNMENT align);

		/**  
		* @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		* @note 获取子项节点列表
		*/
		CExLabelItemNodeList GetChildList();

		/**  
		  * @note 设置扩展节点是否可见
		*/
		virtual void SetVisible(const bool& bVisible);

		/**  
		  * @note 设置绑定点是否贴地
		*/
		void SetClampToTerrain(bool bClamp) { m_bClampToTerrain = bClamp; }

		/**  
		  * @note 判断绑定点是否贴地
		*/
		bool IsClampToTerrain() { return m_bClampToTerrain; }

	protected:
		/**  
		* @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLabelNodeOption* GetOption();

		enum E_LINK_TYPE
		{
			E_LINK_LT = 0,
			E_LINK_LB,
			E_LINK_RT,
			E_LINK_RB
		};

	private:
		/**  
		* @note 初始化Camera结点
		*/
		void CreateCameraNode();

		/**  
		* @note 初始化连接线
		*/
		void CreateLinkLine();

		/**  
		* @note 初始化标头
		*/
		void CreateTitle();

		/**  
		* @note 初始化初始位置
		*/
		void InitPosition();

		/**  
		* @note 设置标牌位置
		*/
		void SetLabelPosition(const osg::Vec2& vecLTNorm);

		/**  
		* @note 根据绑定点计算合适的连接线方式
		*/
		void UpdateLinkTypeByBindingPoint();

		/**  
		* @note 更新连接线
		*/
		void UpdateBindingLine();

		/**  
		* @note 根据绑定点更新标牌
		*/
		void UpdateLabelByBindingPoint();

		/**  
		* @note 校验位置值，使之有效。防止标牌显示到屏幕外
		*/
		void MakePosValid(osg::Vec2d& vecPosNorm);

		/**  
		* @note 查找父节点，第一级菜单父节点为 NULL 
		*/
		CExLabelItemNode* FindParentNode(CExLabelItemNode* pItemNode);

		/**  
		* @note 隐藏兄弟节点树
		*/
		void HideBrotherTree(CExLabelItemNode* pItemNode);

	private:
		/// 相机结点，将屏幕坐标映射到【0，1】，提升效率
		osg::ref_ptr<osg::Camera>       m_rpCameraNode;

		/// 连接线顶点数组
		/// 第一个点是与标牌相连点，为起点。
		/// 第二个点是绑定点，为终点
		osg::ref_ptr<osg::Vec3Array>    m_rpLineVertexes; 

		/// 连接线起始点连接方式
		E_LINK_TYPE                     m_eLinkType;

		/// 绑定点的经纬高坐标
		//osg::Vec3                       m_vecBindingLLH;

		/// 绑定点坐标
		osg::Vec2                       m_vecBindPtNorm;

		/// 连接点坐标
		osg::Vec2                       m_vecLineStartPtNorm;

		/// 是否启用绑定
		bool                            m_bEnableBinding;

		/// 是否显示子项
		bool                            m_bShowItem;

		/// 连接线是否被拉伸，执行了拖拽动作，则该值为true
		bool                            m_bLineChanged;

		/// 是否需要重新绘制
		bool                            m_bNeedRedraw;

		/// 连接线绘制对象
		osg::ref_ptr<osg::Geometry>     m_rpGeomLinkLine;

		/// 标头节点
		osg::ref_ptr<CExLabelItemNode>  m_rpTitleNode;

		/// 子项节点列表，其中第一项为标头节点
		CExLabelItemNodeList            m_listItemNode;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;

		/// 标牌绑定节点回调处理
		osg::ref_ptr<CExLabelBindingNodeCallback>  m_rpLabelBindingCallback;

		/// 是否贴地（为了解决地形高程加载导致位置被裁剪的问题）
		bool                            m_bClampToTerrain;
	};
}


#endif //FE_EXTERN_LABEL_NODE_H
