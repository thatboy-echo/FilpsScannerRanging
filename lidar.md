lidar

| 🗳地址       | 📦参数                                   | 🚢返回值                                | 📎备注                                                       |
| ----------- | --------------------------------------- | -------------------------------------- | ----------------------------------------------------------- |
| /distance   | angle(-45~225)<br/>cid                  | distance<br/>angle                     | 获得指定方向的距离                                          |
| /height     | cid                                     | *                                      | 获得高程数据（90组）                                        |
| ~~/set~~    | ~~device_height<br/>device_y<br/>face~~ | ~~*~~                                  | ~~设置设备相对高度、y轴偏移、方向，此操作会重置服务器地图~~ |
| /coord      |                                         | beginAngle,endAngle,distance[],angle[] | 获得原始坐标数据（无过滤）                                  |
| ~~/line~~   |                                         | ~~beginAngle,endAngle,lines[]~~        | ~~获得所有线段~~                                            |
| /info       | device_list<br/>online_number           | *                                      | 查询在线设备数量和信息                                      |
| /connect    | ip                                      | cid                                    | 连接设备                                                    |
| /disconnect | cid                                     | *                                      | 断开设备                                                    |
| /shutdown   |                                         | *                                      | 断开设备并关闭服务器                                        |
| /quit       |                                         | *                                      | 断开设备并关闭服务器                                        |

```java
package cn.smartion.util;


class Coord
{
    int y;
    int h;
}
public class ReviceRowData
{
    public int begin;
    public int end;
    public int cid;
    public Coord height;
}
```

