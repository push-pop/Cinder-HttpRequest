#include "HttpRequest.h"

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/app/RendererGl.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"
#include "cinder/Font.h"
#include "cinder/gl/Texture.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class HttpRequestSampleApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    void renderText(std::string s);
    
    void performGET();
    void performPOST();
    void performPUT();
    void performDELETE();
    
    params::InterfaceGlRef  params;

    ci::gl::TextureRef mTextTexture;
    ci::Font mFont;
    
    int mNumParams;
    
    std::string mHost;
    std::string mEndpt;
    
    std::string mReqBody;
    
};

void HttpRequestSampleApp::setup()
{
    
    mHost = "www.httpbin.org";
    mEndpt = "/get";
    mReqBody = "{ \"key\": \"value\"} ";
    
    params = params::InterfaceGl::create("HTTP Request Sample", vec2(300,400));
    params->setPosition(vec2(0,0));
    
    params->addParam("Host", &mHost);
    params->addParam("Endpoint", &mEndpt);
    
    params->addText("Perform REST Call: ");
    params->addSeparator();
    params->addButton("GET", std::bind(&HttpRequestSampleApp::performGET, this));
    params->addButton("POST", std::bind(&HttpRequestSampleApp::performPOST, this));
    params->addButton("PUT", std::bind(&HttpRequestSampleApp::performPUT, this));
    params->addButton("DELETE", std::bind(&HttpRequestSampleApp::performDELETE, this));
    
    params->addText("Request arguments: ");
    params->addSeparator();
//    params->addButton("MoreArgs", [](){
//    });
//    params->addButton("Less Args", [](){
//    });
    params->addParam("Request Body", &mReqBody);
    
    
    
#if defined(CINDER_COCOA)
    mFont = Font("Cochin-Italic",14);
#else
    mFont = Font("Times new Roman", 14);
#endif
    

}

void HttpRequestSampleApp::mouseDown( MouseEvent event )
{
}

void HttpRequestSampleApp::update()
{
    //textBox = TextBox().alignment(TextBox::LEFT).size(vec2(400, TextBox::GROW)).text(response);
    //mTextTexture = gl::Texture::create(textBox.render());
    
}

void HttpRequestSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    params->draw();
  if(mTextTexture)
    gl::draw(mTextTexture, vec2(300, 0));

    
    
}

void HttpRequestSampleApp::performGET()
{
    AsyncHttpRequest::create(io_service(), mHost)->BeginGet(mEndpt, [this](std::string resp){
       
        renderText(resp);
    });
}

void HttpRequestSampleApp::performPOST()
{
    JsonTree body;
    try {
        body = JsonTree(mReqBody);
    } catch (Exception e) {
        renderText("Invalid Message Body");
        return;
    }
    
    
    AsyncHttpRequest::create(io_service(), mHost)->BeginPost(mEndpt, body, [this](std::string resp){
        renderText(resp);
        
    });
    
}

void HttpRequestSampleApp::performPUT()
{
    JsonTree body;
    try {
        body = JsonTree(mReqBody);
    } catch (Exception e) {
        renderText("Invalid Message Body");
        return;
    }
    
    AsyncHttpRequest::create(io_service(), mHost)->BeginPut(mEndpt, body, [this](std::string resp){
        renderText(resp);
    });

}

void HttpRequestSampleApp::performDELETE()
{
    JsonTree body;
    try {
        body = JsonTree(mReqBody);
    } catch (Exception e) {
        renderText("Invalid Message Body");
        return;
    }
    
    AsyncHttpRequest::create(io_service(), mHost)->BeginDelete(mEndpt, body, [this](std::string resp){
        renderText(resp);
    });
}

void HttpRequestSampleApp::renderText(std::string s)
{
    
    TextBox textBox = TextBox().size(vec2(300,TextBox::GROW)).text(s);
    mTextTexture = gl::Texture::create( textBox.render());
}


CINDER_APP_NATIVE( HttpRequestSampleApp, RendererGl )
