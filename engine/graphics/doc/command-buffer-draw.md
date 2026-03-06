Direct3D 11 支持以下的绘制命令：

- `ID3D11DeviceContext::Draw`
- `ID3D11DeviceContext::DrawAuto`
- `ID3D11DeviceContext::DrawIndexed`
- `ID3D11DeviceContext::DrawIndexedInstanced`
- `ID3D11DeviceContext::DrawIndexedInstancedIndirect`
- `ID3D11DeviceContext::DrawInstanced`
- `ID3D11DeviceContext::DrawInstancedIndirect`

排除掉不打算支持的间接绘制和自动绘制，只剩下：

- `ID3D11DeviceContext::Draw`
- `ID3D11DeviceContext::DrawIndexed`
- `ID3D11DeviceContext::DrawIndexedInstanced`
- `ID3D11DeviceContext::DrawInstanced`

SDL 3 去掉了无实例化渲染的版本，但这意义不大。
