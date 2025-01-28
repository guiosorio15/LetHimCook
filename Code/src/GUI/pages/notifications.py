import flet as ft

def notifications_page():
    return ft.Container(
        content=ft.Column(
            controls=[
                ft.Text("Notifications", size=30, weight=ft.FontWeight.BOLD),
                ft.Divider(),
                ft.ListView(
                    controls=[
                        ft.ListTile(
                            leading=ft.Icon(ft.icons.PERSON),
                            title=ft.Text(""),
                            subtitle=ft.Text(""),
                        ),
                        ft.ListTile(
                            leading=ft.Icon(ft.icons.VIDEOCAM),
                            title=ft.Text(""),
                        ),
                        ft.ListTile(
                            leading=ft.Icon(ft.icons.PERSON),
                            title=ft.Text(""),
                            subtitle=ft.Text(""),
                        ),
                        ft.ListTile(
                            leading=ft.Icon(ft.icons.SHIELD),
                            title=ft.Text(""),
                        ),
                        ft.ListTile(
                            leading=ft.Icon(ft.icons.BOLT),
                            title=ft.Text(""),
                            subtitle=ft.Text(""),
                        ),
                    ]
                ),
            ],
            alignment=ft.MainAxisAlignment.START,
        ),
        padding=20,
    )
