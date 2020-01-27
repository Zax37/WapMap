#ifndef SHR_GCN_TOOLTIP
#define SHR_GCN_TOOLTIP

#define SHR_TOOLTIP_DELAY 0.5f //in seconds

namespace SHR {
    class cTooltip {
    private:
        char *m_szTooltip;
        float m_fTooltipTimer;
        int m_iTooltipX, m_iTooltipY;
        bool m_bTooltipVisible;
        int m_iTooltipLC;
    public:
        cTooltip();

        ~cTooltip();

        void SetTooltip(const char *n);

        bool HasTooltip() { return (m_szTooltip != 0); };

        bool IsTooltipVisible() { return m_bTooltipVisible; };

        const char *GetTooltip() { return m_szTooltip; };

        void RenderTooltip();

        void UpdateTooltip(bool hasmouse);
    };
}

#endif
