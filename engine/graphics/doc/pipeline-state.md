# 管线状态

## 需要管理的状态

LuaSTG Sub 只用到基本的图形光栅化管线。

如果要封装 `PipelineState`，需要追踪以下状态：

- `ID3D11InputLayout`
- `D3D11_PRIMITIVE_TOPOLOGY`
- `ID3D11BlendState`
- `ID3D11DepthStencilState`
- `ID3D11PixelShader`
- `ID3D11RasterizerState`
- `ID3D11VertexShader`

## 其他管线状态

- `float blend_factor[4]` 提供一个设置方法
- ~~`uint32 sample_mask` 用不上~~
- `uint32 stencil_ref` 提供一个设置方法

## 命令队列/命令列表方法

- `ID3D11DeviceContext::IASetIndexBuffer`
- `ID3D11DeviceContext::IASetInputLayout`
- `ID3D11DeviceContext::IASetPrimitiveTopology`
- `ID3D11DeviceContext::IASetVertexBuffers`
- `ID3D11DeviceContext::OMSetBlendState`
- `ID3D11DeviceContext::OMSetDepthStencilState`
- `ID3D11DeviceContext::OMSetRenderTargets`
- `ID3D11DeviceContext::PSSetConstantBuffers`
- `ID3D11DeviceContext::PSSetSamplers`
- `ID3D11DeviceContext::PSSetShader`
- `ID3D11DeviceContext::PSSetShaderResources`
- `ID3D11DeviceContext::RSSetScissorRects`
- `ID3D11DeviceContext::RSSetState`
- `ID3D11DeviceContext::RSSetViewports`
- `ID3D11DeviceContext::VSSetConstantBuffers`
- `ID3D11DeviceContext::VSSetSamplers`
- `ID3D11DeviceContext::VSSetShader`
- `ID3D11DeviceContext::VSSetShaderResources`
- `ID3D11DeviceContext1::PSSetConstantBuffers1`
- `ID3D11DeviceContext1::VSSetConstantBuffers1`

## 其他命令队列/命令列表方法

这些方法不涉及管线状态，或者不打算支持。

命令队列：

- `ID3D11DeviceContext::ExecuteCommandList`
- `ID3D11DeviceContext::FinishCommandList`

绘制：

- `ID3D11DeviceContext::Draw`
- `ID3D11DeviceContext::DrawAuto`
- `ID3D11DeviceContext::DrawIndexed`
- `ID3D11DeviceContext::DrawIndexedInstanced`
- `ID3D11DeviceContext::DrawIndexedInstancedIndirect`
- `ID3D11DeviceContext::DrawInstanced`
- `ID3D11DeviceContext::DrawInstancedIndirect`

调度：

- `ID3D11DeviceContext::Dispatch`
- `ID3D11DeviceContext::DispatchIndirect`
- `ID3D11DeviceContext::GenerateMips`
- `ID3D11DeviceContext::ResolveSubresource`

设置管线状态：

- `ID3D11DeviceContext::CSSetConstantBuffers`
- `ID3D11DeviceContext::CSSetSamplers`
- `ID3D11DeviceContext::CSSetShader`
- `ID3D11DeviceContext::CSSetShaderResources`
- `ID3D11DeviceContext::CSSetUnorderedAccessViews`
- `ID3D11DeviceContext::DSSetConstantBuffers`
- `ID3D11DeviceContext::DSSetSamplers`
- `ID3D11DeviceContext::DSSetShader`
- `ID3D11DeviceContext::DSSetShaderResources`
- `ID3D11DeviceContext::GSSetConstantBuffers`
- `ID3D11DeviceContext::GSSetSamplers`
- `ID3D11DeviceContext::GSSetShader`
- `ID3D11DeviceContext::GSSetShaderResources`
- `ID3D11DeviceContext::HSSetConstantBuffers`
- `ID3D11DeviceContext::HSSetSamplers`
- `ID3D11DeviceContext::HSSetShader`
- `ID3D11DeviceContext::HSSetShaderResources`
- `ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews`
- `ID3D11DeviceContext::SetResourceMinLOD`
- `ID3D11DeviceContext::SOSetTargets`
- `ID3D11DeviceContext1::CSSetConstantBuffers1`
- `ID3D11DeviceContext1::DSSetConstantBuffers1`
- `ID3D11DeviceContext1::GSSetConstantBuffers1`
- `ID3D11DeviceContext1::HSSetConstantBuffers1`

读取管线状态：

- `ID3D11DeviceContext::CSGetConstantBuffers`
- `ID3D11DeviceContext::CSGetSamplers`
- `ID3D11DeviceContext::CSGetShader`
- `ID3D11DeviceContext::CSGetShaderResources`
- `ID3D11DeviceContext::CSGetUnorderedAccessViews`
- `ID3D11DeviceContext::DSGetConstantBuffers`
- `ID3D11DeviceContext::DSGetSamplers`
- `ID3D11DeviceContext::DSGetShader`
- `ID3D11DeviceContext::DSGetShaderResources`
- `ID3D11DeviceContext::GetResourceMinLOD`
- `ID3D11DeviceContext::GSGetConstantBuffers`
- `ID3D11DeviceContext::GSGetSamplers`
- `ID3D11DeviceContext::GSGetShader`
- `ID3D11DeviceContext::GSGetShaderResources`
- `ID3D11DeviceContext::HSGetConstantBuffers`
- `ID3D11DeviceContext::HSGetSamplers`
- `ID3D11DeviceContext::HSGetShader`
- `ID3D11DeviceContext::HSGetShaderResources`
- `ID3D11DeviceContext::IAGetIndexBuffer`
- `ID3D11DeviceContext::IAGetIndexBuffer`
- `ID3D11DeviceContext::IAGetPrimitiveTopology`
- `ID3D11DeviceContext::IAGetVertexBuffers`
- `ID3D11DeviceContext::OMGetBlendState`
- `ID3D11DeviceContext::OMGetDepthStencilState`
- `ID3D11DeviceContext::OMGetRenderTargets`
- `ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews`
- `ID3D11DeviceContext::PSGetConstantBuffers`
- `ID3D11DeviceContext::PSGetSamplers`
- `ID3D11DeviceContext::PSGetShader`
- `ID3D11DeviceContext::PSGetShaderResources`
- `ID3D11DeviceContext::RSGetScissorRects`
- `ID3D11DeviceContext::RSGetState`
- `ID3D11DeviceContext::RSGetViewports`
- `ID3D11DeviceContext::SOGetTargets`
- `ID3D11DeviceContext::VSGetConstantBuffers`
- `ID3D11DeviceContext::VSGetSamplers`
- `ID3D11DeviceContext::VSGetShader`
- `ID3D11DeviceContext::VSGetShaderResources`
- `ID3D11DeviceContext1::CSGetConstantBuffers1`
- `ID3D11DeviceContext1::DSGetConstantBuffers1`
- `ID3D11DeviceContext1::GSGetConstantBuffers1`
- `ID3D11DeviceContext1::HSGetConstantBuffers1`
- `ID3D11DeviceContext1::PSGetConstantBuffers1`
- `ID3D11DeviceContext1::VSGetConstantBuffers1`

复制：

- `ID3D11DeviceContext::CopyResource`
- `ID3D11DeviceContext::CopyStructureCount`
- `ID3D11DeviceContext::CopySubresourceRegion`
- `ID3D11DeviceContext::Map`
- `ID3D11DeviceContext::Unmap`
- `ID3D11DeviceContext::UpdateSubresource`
- `ID3D11DeviceContext1::CopySubresourceRegion1`
- `ID3D11DeviceContext1::UpdateSubresource1`

清理/填充资源/视图：

- `ID3D11DeviceContext::ClearDepthStencilView`
- `ID3D11DeviceContext::ClearRenderTargetView`
- `ID3D11DeviceContext::ClearUnorderedAccessViewFloat`
- `ID3D11DeviceContext::ClearUnorderedAccessViewUint`
- `ID3D11DeviceContext1::ClearView`
- `ID3D11DeviceContext1::DiscardResource`
- `ID3D11DeviceContext1::DiscardView`
- `ID3D11DeviceContext1::DiscardView1`

查询：

- `ID3D11DeviceContext::Begin`
- `ID3D11DeviceContext::End`
- `ID3D11DeviceContext::GetData`
- `ID3D11DeviceContext::GetPredication`
- `ID3D11DeviceContext::SetPredication`

同步：

- `ID3D11DeviceContext::Flush`
- `ID3D11DeviceContext3::Flush1`
- `ID3D11DeviceContext4::Signal`
- `ID3D11DeviceContext4::Wait`

内容保护：

- `ID3D11DeviceContext3::GetHardwareProtectionState`
- `ID3D11DeviceContext3::SetHardwareProtectionState`

分块资源：

- `ID3D11DeviceContext2::CopyTileMappings`
- `ID3D11DeviceContext2::CopyTiles`
- `ID3D11DeviceContext2::ResizeTilePool`
- `ID3D11DeviceContext2::TiledResourceBarrier`
- `ID3D11DeviceContext2::UpdateTileMappings`
- `ID3D11DeviceContext2::UpdateTiles`

其他：

- `ID3D11DeviceContext::ClearState`
- `ID3D11DeviceContext::GetContextFlags`
- `ID3D11DeviceContext::GetType`

兼容层：

- `ID3D11DeviceContext1::SwapDeviceContextState`

图形调试：

- `ID3D11DeviceContext2::BeginEventInt`
- `ID3D11DeviceContext2::EndEvent`
- `ID3D11DeviceContext2::IsAnnotationEnabled`
- `ID3D11DeviceContext2::SetMarkerInt`
