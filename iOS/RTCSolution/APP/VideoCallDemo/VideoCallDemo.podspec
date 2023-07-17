
Pod::Spec.new do |spec|
  spec.name         = 'VideoCallDemo'
  spec.version      = '1.0.0'
  spec.summary      = 'VideoCallDemo APP'
  spec.description  = 'VideoCallDemo App Demo..'
  spec.homepage     = 'https://github.com/volcengine'
  spec.license      = { :type => 'MIT', :file => 'LICENSE' }
  spec.author       = { 'author' => 'volcengine rtc' }
  spec.source       = { :path => './'}
  spec.ios.deployment_target = '9.0'
  
  spec.source_files = '**/*.{h,m,c,mm}'
  spec.resource_bundles = {
    'VideoCallDemo' => ['Resource/*.{xcassets,bundle}']
  }
  spec.pod_target_xcconfig = {'CODE_SIGN_IDENTITY' => ''}
  spec.prefix_header_contents = '#import "Masonry.h"',
                                '#import "ToolKit.h"',
                                '#import "VideoCallDemoConstants.h"'
  spec.dependency 'ToolKit'
  spec.dependency 'YYModel'
  spec.dependency 'Masonry'
end
